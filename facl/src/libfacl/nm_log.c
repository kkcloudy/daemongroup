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
* nm_log.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

/* nm_log.c */
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "nm_list.h"
#include "nm_log.h"
#include "nm_errcode.h"
#include "nm_blkmem.h"

int log_forward = 0;
unsigned int nm_log_level = 0xFF;		/* all */
unsigned int nm_daemon_log_open = NM_DAEMON_LOG_CLOSE;   /* close */
char STR_NM[32] = {0}; 				/* syslog string nm	*/

#define NM_LOG_BLEMEM_MAX_ITEM_NUM		64
#define NM_LOG_BLEMEM_MAX_NUM			16

#define MAX_FILTER_LEN	120

struct LogFilterItem {
	struct list_head node;
	char filter[MAX_FILTER_LEN];
};

struct list_head filter_head = LIST_HEAD_INIT(filter_head);
static nm_blk_mem_t *filter_blkmem = NULL;

const char *
safe_strerror(int errnum)
{
	const char *s = strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}

#if 0
int
nm_log_init(int hansitype, int insid)
#else
int
nm_log_init(char *daemon)
#endif
{
	int iret = 0;
	if( NULL == daemon ){
		return NM_ERR_INPUT_PARAM_ERR;
	}
	static char prefix[32];/*it must be static. because openlog not copy the str but save the pointer!!!*/
	strncpy(prefix, daemon, sizeof(prefix) - 1);
	
	nm_log_get_openlog_name(daemon);/* the name will be used for openlog func next time!!!*/
	if (NM_DAEMON_LOG_OPEN != nm_daemon_log_open) {
		openlog(prefix, LOG_PID, LOG_DAEMON);
		setlogmask(LOG_AT_LEAST_DEBUG);
		iret = nm_blkmem_create(&filter_blkmem,
					 "nm_log_filter_blkmem",
					 sizeof (struct LogFilterItem),
					 NM_LOG_BLEMEM_MAX_ITEM_NUM,
					 NM_LOG_BLEMEM_MAX_NUM);
		if (NM_RETURN_OK != iret) {
			syslog(LOG_ERR,
			       "nm_log_init create nm_log_filter_blkmem failed case %s",
			       nm_errcode_content(iret));
			return NM_LOG_ERR_INIT_BLKMEM_FAILED;
		}
		nm_daemon_log_open = NM_DAEMON_LOG_OPEN;
	}
	syslog(LOG_INFO, "%s log inited!", daemon);

	return NM_RETURN_OK;
}

int
nm_log_uninit()
{
	struct LogFilterItem *pos = NULL;
	struct LogFilterItem *n = NULL;
	log_forward = 0;
	nm_daemon_log_open = NM_DAEMON_LOG_CLOSE;
	/*release list */
	list_for_each_entry_safe(pos, n, &filter_head, node) {
		list_del(&(pos->node));
		nm_blkmem_free_item(filter_blkmem, pos);
	}

	/*release filter_blkmem */
	nm_blkmem_destroy(&filter_blkmem);

	return NM_RETURN_OK;
}

void
nm_log_set_level(int level)
{
	setlogmask(level);
	return;
}

static int
nm_log_add_filter_single(char *filter)
{
	int iret = NM_ERR_UNKNOWN;

	struct LogFilterItem *pfilteritem = NULL;
	struct LogFilterItem *pos = NULL;

	if (NULL == filter || strlen(filter)==0) {
		return NM_ERR_INPUT_PARAM_ERR;
	}
	if (strlen(filter) > sizeof (pfilteritem->filter) - 1) {
		return NM_LOG_ERR_ADD_FILTER_OVER_LENGTH;
	}
	/*find the same filter */
	list_for_each_entry(pos, &filter_head, node) {
		int cmp = strcmp(filter, pos->filter);
		if (0 == cmp) {
			return NM_LOG_ERR_ADD_FILTER_ALREADY_HAVE;
		}
		if ( cmp < 0 ){
			break;
		}
	}

	pfilteritem = nm_blkmem_malloc_item(filter_blkmem);
	if (NULL != pfilteritem) {
		memset(pfilteritem, 0, sizeof (struct LogFilterItem));
		if( NULL == pos ){
			list_add_tail(&(pfilteritem->node), &filter_head);
		}else{
			list_add(&(pfilteritem->node), &(pos->node));/*add to first! the filter is be sorted!*/
		}
		strncpy(pfilteritem->filter, filter,
				sizeof (pfilteritem->filter) - 1);
		iret = NM_RETURN_OK;
	} else {
		iret = NM_LOG_ERR_ADD_FILTER_BLK_MALLOC_FAILED;
	}

	return iret;
}

int
nm_log_add_filter(char *filter_mult)
{				/*返回成功添加的filter的个数 */
	char *filter = NULL;
	int num = 0;
	int iret_temp = NM_ERR_UNKNOWN;
	char *filter_mult_temp;

	if (NULL == filter_mult) {
		return 0;
	}

	filter_mult_temp = strdup(filter_mult);
	if (NULL == filter_mult_temp) {
		return 0;
	}

	for (filter = strtok(filter_mult_temp, ":");
	     NULL != filter; filter = strtok(NULL, ":")) {
		iret_temp = nm_log_add_filter_single(filter);
		if (NM_RETURN_OK == iret_temp) {
			num++;
		}
	}

	free(filter_mult_temp);

	return num;
}

static int
nm_log_del_filter_single(char *filter)
{
	struct LogFilterItem *pos, *n;
	int iret = NM_LOG_ERR_FILER_NOT_FIND;

	list_for_each_entry_safe(pos, n, &filter_head, node) {
		if (0 == strcmp(filter, pos->filter)) {
			list_del(&(pos->node));
			iret = nm_blkmem_free_item(filter_blkmem, pos);
			break;
		}
	}

	return iret;
}

int
nm_log_del_filter(char *filter_mult)
{
	char *filter = NULL;
	int num = 0;
	int iret = NM_ERR_UNKNOWN;

	char *filter_mult_temp = NULL;

	if (NULL == filter_mult) {
		return 0;
	}

	filter_mult_temp = strdup(filter_mult);
	if (NULL == filter_mult_temp) {
		return 0;
	}

	for (filter = strtok(filter_mult_temp, ":");
	     NULL != filter; filter = strtok(NULL, ":")) {
		iret = nm_log_del_filter_single(filter);
		if (NM_RETURN_OK == iret) {
			num++;
		}

	}
	free(filter_mult_temp);
	filter_mult_temp = NULL;
	
	return num;
}

static int
nm_log_if_match(const char *match, const char *filter)
{				/*when filter is substr of match */
	char *pmatch = NULL;
	unsigned int filter_len = 0;

	filter_len = strlen(filter);
	pmatch = strstr(match, filter);
	
	if (NULL == pmatch) {
		return NM_FALSE;
	}

	if (':' == pmatch[filter_len] || '\0' == pmatch[filter_len]) {
//		printf("nm_log_if_match match=%s  filter=%s\n",match,filter);
//		printf("nm_log_if_match NM_TRUE\n");
		return NM_TRUE;
	}
	return NM_FALSE;
}

int
nm_log_is_filter_register(const char *match)
{
	struct LogFilterItem *pos;
	int iret = NM_FALSE;

	list_for_each_entry(pos, &filter_head, node) {
		iret = nm_log_if_match(match, pos->filter);
		if (NM_TRUE == iret) {
			break;
		}
	}

	return iret;
}

int 
nm_log_set_debug_value(unsigned int val_mask)
{	
	if (nm_log_level & val_mask) {
		return NM_LOG_ERR_ALREADY_SET;
	}
	
	nm_log_level |= val_mask;

	return NM_RETURN_OK;
}

int 
nm_log_set_no_debug_value(unsigned int val_mask)
{	
	if (nm_log_level & val_mask) {
		nm_log_level &= ~val_mask;
		return NM_RETURN_OK;
	}
	else 
		return NM_LOG_ERR_ALREADY_SET;
}

void nm_log_get_openlog_name(char *daemon)
{
	strncpy(STR_NM, daemon, sizeof(STR_NM)-1);
	syslog(LOG_INFO, "nm_log_get_openlog_name STR_NM = %s", STR_NM);
	
	return;
}
int is_print_log(int level)
{
	if (!(nm_log_level & (LOG_MASK(level)))) {
		return NM_LOG_ERR_LEVEL_NOT_MATCH;	
	}
	
	if (NM_DAEMON_LOG_OPEN != nm_daemon_log_open) {
		openlog(STR_NM, LOG_PID, LOG_DAEMON);
		nm_daemon_log_open = NM_DAEMON_LOG_OPEN;
	}

	return NM_RETURN_OK;
}

#if nm_log_test
#include <stdio.h>
#include <string.h>
#include "nm_mem.c"
#include "nm_blkmem.c"
#include "nm_errcode.c"

int
main()
{
	int iret = NM_RETURN_OK;
	iret = nm_log_init(1);
	if (NM_RETURN_OK != iret) {
		printf("nm_log_init failed :%s\n", nm_errcode_content(iret));
		return 0;
	}

	iret = nm_log_add_filter("test1");
	if (0 == iret) {
		printf("nm_log_add_filter failed test1\n");
	}

	iret = nm_log_add_filter("test1");
	if (0 == iret) {
		printf("nm_log_add_filter failed test1  xxxx\n");
	}

	iret = nm_log_add_filter("test3");
	if (0 == iret) {
		printf("nm_log_add_filter failed test3\n");
	}

	nm_log_syslog(LOG_DEBUG, "test1", "test1");
	nm_log_syslog(LOG_DEBUG, "test", "test");
	nm_log_syslog(LOG_DEBUG, "test2", "test2");
	nm_log_syslog(LOG_DEBUG, "test4", "test4");

	nm_log_debug("test3", "test3");

	nm_log_set_level(LOG_AT_LEAST_NOTICE);

	nm_log_debug("test1", "nm_log_debug  test1");
	nm_log_info("nm_log_info  test1");
	nm_log_notice("nm_log_notice  test1");
	nm_log_warning("nm_log_warning  test1");
	nm_log_err("nm_log_err  test1");

	nm_log_debug("test", "nm_log_debug  test");
	nm_log_info("nm_log_info  test");
	nm_log_notice("nm_log_notice  test");
	nm_log_warning("nm_log_warning  test");
	nm_log_err("nm_log_err  test");

	nm_log_debug("test:test1", "nm_log_debug  test:test1");
	nm_log_info("nm_log_info  test:test1");
	nm_log_notice("nm_log_notice  test:test1");
	nm_log_warning("nm_log_warning  test:test1");
	nm_log_err("nm_log_err  test:test1");

	iret = nm_log_add_filter("test3:test7:test8:test9");
	if (0 == iret) {
		printf("nm_log_init failed\n");
		return 0;
	}

	{
		struct LogFilterItem *pos;
		list_for_each_entry(pos, &filter_head, node) {
			printf("filter : %s\n", pos->filter);
		}
	}

	iret = nm_log_del_filter("test3:test7:test8:test9");
	if (iret != 4) {
		printf("nm_log_del_filter failed iret =%d\n", iret);
	}

	printf("after del!!!!!\n");
	{
		struct LogFilterItem *pos;
		list_for_each_entry(pos, &filter_head, node) {
			printf("filter : %s\n", pos->filter);
		}
	}

	nm_log_debug("test:test1", "after   dell");
	nm_log_debug("test:test1", "nm_log_err  test:test1");
	nm_log_debug("test:test2", "nm_log_err  test:test2");
	nm_log_debug("test:test3", "nm_log_err  test:test3");
	nm_log_debug("test1:test3", "nm_log_err  test1:test3");

	nm_log_uninit();

	return 0;
}

#endif
