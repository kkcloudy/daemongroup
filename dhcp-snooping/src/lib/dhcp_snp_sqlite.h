#ifndef __NPD_DHCP_SNP_SQLITE_H__
#define __NPD_DHCP_SNP_SQLITE_H__

/*********************************************************
*	head files														*
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "sysdef/npd_sysdef.h"
#include "dhcp_snp_tbl.h"
#include "dhcp_snp_com.h"


/*********************************************************
*	macro define													*
**********************************************************/

/*
#define DHCP_SNP_SQLITE3
*/
#define NPD_DHCP_SNP_SQLITE3_DBNAME					"/mnt/dhcpsnp.db"



/*********************************************************
*	struct define													*
**********************************************************/


/*********************************************************
*	function declare												*
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
);

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
);

/**********************************************************************************
 *dhcp_snp_db_insert ()
 *
 *	DESCRIPTION:
 *		insert a item to dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		cNPD_DHCP_SNP_TBL_ITEM_T *item 	- item which need insert into dhcp snp db
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
);

/**********************************************************************************
 *dhcp_snp_db_query ()
 *
 *	DESCRIPTION:
 *		insert a item to dhcp snp db
 *
 *	INPUTS:
 *		sqlite3 *db,
 *		char *sql								- sql of insert a item to dhcp snp db
 *	 	unsigned int count,					- count of records
 *		NPD_DHCP_SNP_SHOW_ITEM_T array[]	- array of records
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
);

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
);

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
);

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
);

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
);


/*********************************************************
*	extern Functions												*
**********************************************************/

#endif

