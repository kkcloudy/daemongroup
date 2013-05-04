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
* dhcp_snp_sqlite.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping sqlite for NPD module.
*
* DATE:
*		06/08/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.1.1.1 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include <sysdef/returncode.h>

#include "dhcp_snp_log.h"
#include "dhcp_snp_sqlite.h"

/*********************************************************
*	global variable define											*
**********************************************************/


/*********************************************************
*	extern variable												*
**********************************************************/


/*********************************************************
*	functions define												*
**********************************************************/
/**********************************************************************************
 *dhcp_snp_db_open ()
 *
 *	DESCRIPTION:
 *		open dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 **db		- dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OPEN_DB_FAIL	- open dhcp snp db fail
 *		DHCP_SNP_RETURN_CODE_OK			- open dhcp snp db success
 ***********************************************************************************/
unsigned int dhcp_snp_db_open
(
	sqlite3 **db
)
{
#ifdef DHCP_SNP_SQLITE3

	int rc = 0;
	sqlite3 *tmp_db = NULL;

	rc = sqlite3_open(NPD_DHCP_SNP_SQLITE3_DBNAME, &tmp_db);
	if (rc != SQLITE_OK || tmp_db == NULL) {
		dhcp_snp_db_close(tmp_db);
		syslog_ax_dhcp_snp_err("open %s error, rc %x\n", NPD_DHCP_SNP_SQLITE3_DBNAME, rc);
		return DHCP_SNP_RETURN_CODE_OPEN_DB_FAIL;
	}

	*db = tmp_db;
	syslog_ax_dhcp_snp_dbg("open %s %p success.\n", NPD_DHCP_SNP_SQLITE3_DBNAME, *db);
#endif
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_db_create ()
 *
 *	DESCRIPTION:
 *		create dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		char *sql		- sql of create dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_db_create
(
	sqlite3 *db,
	char *sql
)
{
#ifdef DHCP_SNP_SQLITE3
	int rc = 0;
    char * zErrMsg = 0;

    if (!sql || !db) {
		syslog_ax_dhcp_snp_err("create db with null message!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc == SQLITE_OK) {
		syslog_ax_dhcp_snp_dbg("create dhcpSnpDb success.\n");
	}else if (rc == SQLITE_ERROR) {
		syslog_ax_dhcp_snp_dbg("%s, rc %x.\n", zErrMsg ? zErrMsg : "", rc);
	}else {
		syslog_ax_dhcp_snp_err("%s, rc %x!\n", zErrMsg ? zErrMsg : "", rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
#endif	

    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_db_insert ()
 *
 *	DESCRIPTION:
 *		insert a item to dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 	- item which need insert into dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_db_insert
(
	sqlite3 *db,
	NPD_DHCP_SNP_TBL_ITEM_T *item 
)
{
#ifdef DHCP_SNP_SQLITE3

	int rc = 0;
	char * zErrMsg = NULL;

	char zInsertItem[1024] = {0};
	char *bufPtr = NULL;
	int loglength = NPD_DHCP_SNP_INIT_0;

    if (!item || !db) {
		syslog_ax_dhcp_snp_err("insert db with null message\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	/* table struct:
	 *	MAC 		VARCHAR(12) NOT NULL UNIQUE,\
	 *	IP 			INTEGER NOT NULL UNIQUE,\
	 *	VLAN 		INTEGER,\
	 *	PORT 		INTEGER,\
	 *	SYSTIME	 	INTEGER,\
	 *	LESAE 		INTEGER,\
	 *	BINDTYPE	INTEGER
	 */
	/* insert the item to dhcp snp db*/
	bufPtr = zInsertItem;

	loglength += sprintf(bufPtr, "INSERT INTO dhcpSnpDb VALUES('");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%02x", item->chaddr[0]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[1]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[2]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[3]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[4]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[5]);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "',");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->ip_addr);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->vlanId);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->ifindex);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->sys_escape);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->lease_time);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->bind_type);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ");");
	bufPtr = zInsertItem + loglength;

	syslog_ax_dhcp_snp_dbg("%s\n", zInsertItem);

	rc = sqlite3_exec(db, zInsertItem, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		syslog_ax_dhcp_snp_err("%s%s, rc %x!\n",
								zErrMsg ? "error:" : "",
								zErrMsg ? zErrMsg : "",
								rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("insert to dhcpSnpDb success.\n");

#endif
	
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_db_query ()
 *
 *	DESCRIPTION:
 *		insert a item to dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		char *sql										- sql of insert a item to dhcp snp db
 *	 	unsigned int count,							- count of records
 *		NPD_DHCP_SNP_SHOW_ITEM_T array[]			- array of records
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_db_query
(
	sqlite3 *db,
	char *sql,
	unsigned int *count,
	NPD_DHCP_SNP_SHOW_ITEM_T array[]
)
{
#ifdef DHCP_SNP_SQLITE3
	int rc = 0;
	int i = NPD_DHCP_SNP_INIT_0;
	int j = NPD_DHCP_SNP_INIT_0;
	int nRow = NPD_DHCP_SNP_INIT_0;
	int nColumn = NPD_DHCP_SNP_INIT_0;
	char **azResult = NULL;
	char *zErrMsg = NULL;

    if (!sql || !db || !array) {
		syslog_ax_dhcp_snp_err("sql query with null message\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	syslog_ax_dhcp_snp_dbg("%s\n", sql);
	syslog_ax_dhcp_snp_dbg("bef select:\n");

	rc = sqlite3_get_table(db, sql, &azResult, &nRow, &nColumn, &zErrMsg);
	if (rc != SQLITE_OK && rc != SQLITE_NOTFOUND) {
		syslog_ax_dhcp_snp_dbg("error select:\n");
		syslog_ax_dhcp_snp_err("%s%s, rc %x!\n",
								zErrMsg ? "error:" : "",
								zErrMsg ? zErrMsg : "",
								rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("after select:\n");

	/* for debug */
    syslog_ax_dhcp_snp_dbg("query result %d row(s) %d column(s):\n#",nRow, nColumn);
	for (i = 0; i <= nRow; i++) {
		for(j = 0; j < nColumn; j++) {
			syslog_ax_dhcp_snp_dbg("%-12s ", azResult[i * nColumn + j]);
		}
		syslog_ax_dhcp_snp_dbg("#\n#");
    }
	
	for (i = 1; i <= nRow; i++) {
		/* BINDTYPE, IP, MAC, LESAE, VLAN, PORT */
		/******************************************************************/
		array[i - 1].bind_type  = strtoul(azResult[i * nColumn + 0], (char **)NULL, 10);
		array[i - 1].ip_addr    = strtoul(azResult[i * nColumn + 1], (char **)NULL, 10);
		memcpy((char *)(array[i-1].chaddr), azResult[i * nColumn + 2], 12);
		array[i - 1].lease_time = strtoul(azResult[i * nColumn + 3], (char **)NULL, 10);
		array[i - 1].vlanId     = strtoul(azResult[i * nColumn + 4], (char **)NULL, 10);
		array[i - 1].ifindex    = strtoul(azResult[i * nColumn + 5], (char **)NULL, 10);

	    syslog_ax_dhcp_snp_dbg("bind %d ip %3d.%3d.%3d.%3d mac %c%c%c%c%c%c%c%c%c%c%c%c lease %d vlanid %d ifindex %d\n",
								array[i - 1].bind_type,
								(array[i - 1].ip_addr>>24) & 0xff, (array[i - 1].ip_addr>>16) & 0xff,
								(array[i - 1].ip_addr>>8) & 0xff, array[i - 1].ip_addr & 0xff,
								array[i-1].chaddr[0], array[i-1].chaddr[1], array[i-1].chaddr[2], array[i-1].chaddr[3], 
								array[i-1].chaddr[4], array[i-1].chaddr[5], array[i-1].chaddr[6], array[i-1].chaddr[7], 
								array[i-1].chaddr[8], array[i-1].chaddr[9], array[i-1].chaddr[10], array[i-1].chaddr[11], 
								array[i - 1].lease_time,
								array[i - 1].vlanId, array[i - 1].ifindex);
    }

	*count = nRow;

    sqlite3_free_table(azResult);

	syslog_ax_dhcp_snp_dbg("query from dhcpSnpDb success.\n");
#endif
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_db_update ()
 *
 *	DESCRIPTION:
 *		update a item to dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 	- item of update to dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_db_update
(
	sqlite3 *db,
	NPD_DHCP_SNP_TBL_ITEM_T *item 
)
{
#ifdef DHCP_SNP_SQLITE3
	int rc = 0;
    char * zErrMsg = NULL;

	char zUpdateItem[1024] = {0};
	char *bufPtr = NULL;
	int loglength = NPD_DHCP_SNP_INIT_0;

    if (!item || !db) {
		syslog_ax_dhcp_snp_err("sql update with null message\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	/* update the item to dhcp snp db*/
	bufPtr = zUpdateItem;

	loglength += sprintf(bufPtr, "UPDATE dhcpSnpDb SET ");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "IP=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->ip_addr);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, ",SYSTIME=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->sys_escape);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, ",LEASE=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->lease_time);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, " WHERE MAC='");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%02x", item->chaddr[0]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[1]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[2]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[3]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[4]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[5]);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "';");
	bufPtr = zUpdateItem + loglength;
	
	syslog_ax_dhcp_snp_dbg("%s\n", zUpdateItem);

    rc = sqlite3_exec(db, zUpdateItem, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		syslog_ax_dhcp_snp_err("%s%s, rc %x!\n",
								zErrMsg ? "error:" : "",
								zErrMsg ? zErrMsg : "",
								rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("update to dhcpSnpDb success.\n");
#endif
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_db_delete ()
 *
 *	DESCRIPTION:
 *		delete a item to dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		NPD_DHCP_SNP_TBL_ITEM_T *item	- item which'l delete from dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
**********************************************************************************/
unsigned int dhcp_snp_db_delete
(
	sqlite3 *db,
	NPD_DHCP_SNP_TBL_ITEM_T *item 
)
{
#ifdef DHCP_SNP_SQLITE3
	int rc = 0;
    char * zErrMsg = NULL;

	char zDeleteItem[1024] = {0};
	char *bufPtr = NULL;
	int loglength = NPD_DHCP_SNP_INIT_0;

    if (!item || !db) {
		syslog_ax_dhcp_snp_err("delete db with null message\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	/* delete the item from dhcp snp db*/
	bufPtr = zDeleteItem;

	loglength += sprintf(bufPtr, "DELETE FROM dhcpSnpDb WHERE MAC='");
	bufPtr = zDeleteItem + loglength;

	loglength += sprintf(bufPtr, "%02x", item->chaddr[0]);
	bufPtr = zDeleteItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[1]);
	bufPtr = zDeleteItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[2]);
	bufPtr = zDeleteItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[3]);
	bufPtr = zDeleteItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[4]);
	bufPtr = zDeleteItem + loglength;
	loglength += sprintf(bufPtr, "%02x", item->chaddr[5]);
	bufPtr = zDeleteItem + loglength;

	loglength += sprintf(bufPtr, "';");
	bufPtr = zDeleteItem + loglength;

	log_debug("%s\n", zDeleteItem);

    rc = sqlite3_exec(db, zDeleteItem, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		syslog_ax_dhcp_snp_err("%s%s, rc %x!\n",
								zErrMsg ? "error:" : "",
								zErrMsg ? zErrMsg : "",
								rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	log_debug("delete from dhcpSnpDb success.\n");

#endif
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_db_drop ()
 *
 *	DESCRIPTION:
 *		drop dhcp snp table
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		char *sql		- sql of create dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL - sql is null
 *		DHCP_SNP_RETURN_CODE_OK 		- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_db_drop
(
	sqlite3 *db,
	char *sql
)
{
#ifdef DHCP_SNP_SQLITE3
	int rc = 0;
	char * zErrMsg = 0;

	if (!sql || !db) {
		syslog_ax_dhcp_snp_err("drop db with null message\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		syslog_ax_dhcp_snp_err("%s, rc %x!\n", zErrMsg ? zErrMsg : "", rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("drop table dhcpSnpDb success.\n");
#endif
	return DHCP_SNP_RETURN_CODE_OK;
}



/**********************************************************************************
 *dhcp_snp_db_update ()
 *
 *	DESCRIPTION:
 *		update a item to dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
**********************************************************************************/
void dhcp_snp_db_close
(
	sqlite3 *db
)
{
#ifdef DHCP_SNP_SQLITE3
	int rc = 0;

    if (!db) {
		syslog_ax_dhcp_snp_err("close db with null message!\n");
		return ;
    }

    rc = sqlite3_close(db);
    if (SQLITE_OK != rc) {
		syslog_ax_dhcp_snp_err("close %s fail, rc %x!\n", NPD_DHCP_SNP_SQLITE3_DBNAME, rc);
		return ;
	}

	syslog_ax_dhcp_snp_dbg("close %s success.\n", NPD_DHCP_SNP_SQLITE3_DBNAME);
#endif
    return ;
}


#ifdef __cplusplus
}
#endif

