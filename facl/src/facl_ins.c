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
* facl_ins.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* facl ins
*
*
*******************************************************************************/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "nm_list.h"
#include "nm_mem.h"
#include "nm_log.h"
#include "nm_blkmem.h"
#include "nm_dbus.h"
#include "nm_thread.h"
#include "facl_errcode.h"
#include "facl_command.h"
#include "facl_ins.h"

#if 0
const char FACL_DBUS_NAME[MAX_DBUS_BUSNAME_LEN] = "aw.nm_facl";
const char FACL_DBUS_OBJPATH[MAX_DBUS_BUSNAME_LEN] = "/aw/nm_facl";
const char FACL_DBUS_INTERFACE[MAX_DBUS_BUSNAME_LEN] = "aw.nm_facl";
#endif

struct facl_ins {
	int policy_num;
	facl_db_t *facldb;
	nm_thread_master_t *master;
	nm_dbus_t *dbus;
};

facl_ins_t *
facl_ins_new(void)
{
	facl_ins_t *faclins = NULL;

	faclins = nm_malloc(sizeof(*faclins));
	if (NULL == faclins) {
		nm_log_err("facl_ins_new nm_malloc failed");
		goto failed_0;
	}

	memset(faclins, 0, sizeof(*faclins));
	/* thread_master */
	faclins->master = nm_thread_master_new();
	if (NULL == faclins->master) {
		nm_log_err("facl_ins_new nm_thread_master_new failed");
		goto failed_1;
	}
	/*dbus*/
	faclins->dbus = nm_dbus_new(FACL_DBUS_NAME, FACL_DBUS_OBJPATH);
	if (NULL == faclins->dbus) {
		nm_log_err("facl_ins_new nm_dbus_new failed");
		goto failed_2;
	}
	/*db*/
	faclins->facldb = facl_db_create();
	if (NULL == faclins->facldb) {
		nm_log_err("facl_ins_new facl_db_create failed");
		goto failed_3;
	}
	
	faclins_register_all_dbus_method(faclins);

	return faclins;
	
failed_3:
	nm_dbus_free(faclins->dbus);
	faclins->dbus = NULL;
failed_2:
	nm_thread_master_free(faclins->master);
	faclins->master = NULL;
failed_1:
	nm_free(faclins);
	faclins = NULL;
failed_0:
	return NULL;
}

int
facl_ins_free(facl_ins_t *faclins)
{
	if (NULL == faclins) {
		nm_log_err("facl_ins_free input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	
	if (NULL != faclins->dbus) {
		nm_dbus_free(faclins->dbus);
		faclins->dbus = NULL;
	}
	
	if (NULL != faclins->master) {
		nm_thread_master_free(faclins->master);
		faclins->master = NULL;
	}

	if (NULL != faclins->facldb) {
		facl_db_destroy(faclins->facldb);
		faclins->facldb = NULL;
	}

	nm_free(faclins);
	
	return FACL_RETURN_OK;
}

int
facl_ins_start(facl_ins_t *faclins)
{
	
	return FACL_RETURN_OK;
}

int
facl_ins_stop()
{

	return FACL_RETURN_OK;
}

int 
facl_ins_dispatch(facl_ins_t *faclins)
{
	struct timeval timer_wait = {0};

	if (NULL == faclins) {
		nm_log_err("facl_ins_dispatch input error");
		return -1;
	}
	
	timer_wait.tv_sec = 0;
	timer_wait.tv_usec = 10000;
	nm_thread_dispatch(faclins->master, &timer_wait);
	nm_dbus_dispach(faclins->dbus, 0);

	return 0;
}

facl_db_t *
facl_ins_get_db(facl_ins_t *faclins)
{
	if (NULL == faclins) {
		nm_log_err("facl_ins_get_db input error");
		return NULL;
	}

	return faclins->facldb;
}

nm_dbus_t *
facl_ins_get_dbus(facl_ins_t *faclins)
{
	if (NULL == faclins) {
		nm_log_err("facl_ins_get_dbus input error");
		return NULL;
	}

	return faclins->dbus;
}

