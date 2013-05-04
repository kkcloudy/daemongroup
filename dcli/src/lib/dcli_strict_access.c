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
* dcli_strict_access.c
*
*
* CREATOR:
* chensheng@autelan.com
*
* DESCRIPTION:
* CLI definition for strict access module.
*
*
*******************************************************************************/

#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "vty.h"
#include "command.h"

#include "ac_manage_def.h"
#include "ws_firewall.h"
#include "ac_manage_firewall_interface.h"
#include "dcli_main.h"
#include "dcli_strict_access.h"

struct cmd_node strict_access_node =
{
	STRICT_ACCESS_NODE,
	"%s(config-strict-access)# "
};

extern int is_distributed;
extern int is_active_master;

DEFUN(conf_strict_access_func,
	conf_strict_access_cmd,
	"config strict-access",
	CONFIG_STR
	"config strict-access\n"
)
{
	if(CONFIG_NODE == vty->node) {
		vty->node = STRICT_ACCESS_NODE;
		vty->index = NULL;
	}
	else {
		vty_out(vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(config_strict_access_level_func,
	config_strict_access_level_cmd,
	"config strict-access level <0-1>",
	CONFIG_STR
	"config strict-access\n"
	"config strict-access level\n"
	"0 for stop, 1 for start\n"
)
{
	if((is_distributed == 1) && (is_active_master == 0))
	{
		vty_out(vty,"only active master can config strict-access\n");
		return CMD_WARNING;
	}
	
	int level = 0;
	int i = 0;
	
	level = atoi(argv[0]);
	for(i = 1; i < MAX_SLOT; i++) {
		if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 
			continue;
		
		int ret = 0;
		ret = ac_manage_config_strict_access_level(dbus_connection_dcli[i]->dcli_dbus_connection, level);
		switch(ret) {
			case AC_MANAGE_SUCCESS:
				break;
			case AC_MANAGE_DBUS_ERROR:
				vty_out(vty, "Slot(%d): <error> failed get reply.\n", i);
				break;
			default:
				vty_out(vty, "Slot(%d): unknow fail reason:%d\n", i, ret);
				break;
		}
	}	
	
	return CMD_SUCCESS;
}



DEFUN(show_strict_access_config_func,
	show_strict_access_config_cmd,
	"show strict-access config",
	SHOW_STR
	"show strict-access config\n"
	"show strict-access config\n"
)
{
	int level = 0;
	int ret = 0;

	vty_out(vty, "========================================\n");
	ret = ac_manage_show_strict_access(dcli_dbus_connection, &level);
	if (AC_MANAGE_SUCCESS == ret) {
		vty_out(vty, "config strict-access level %d\n", level);
	} else if (AC_MANAGE_DBUS_ERROR == ret) {
		vty_out(vty, "<error> failed get reply.\n");
	} else {
		vty_out(vty, "unknow fail reason: %d\n", ret);
	}
	vty_out(vty, "========================================\n");
	
	return CMD_SUCCESS;
}

static int 
dcli_strict_access_show_running_config(struct vty* vty)
{
	do {
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "strict-access");
		vtysh_add_show_string(_tmpstr);
	} while(0);

	int level = 0;
	int ret = 0;
	
	ret = ac_manage_show_strict_access(dcli_dbus_connection, &level);

	if (AC_MANAGE_SUCCESS == ret && 0 != level) {
		vtysh_add_show_string("config strict-access");

		vtysh_add_show_string(" config strict-access level 1");
		
		vtysh_add_show_string(" exit");
	}
	
	return CMD_SUCCESS;
}

void dcli_strict_access_init(void)
{
	install_node(&strict_access_node, dcli_strict_access_show_running_config, "STRICT_ACCESS_NODE");
	install_default(STRICT_ACCESS_NODE);
	
	install_element(CONFIG_NODE, &conf_strict_access_cmd);

	install_element(CONFIG_NODE, &config_strict_access_level_cmd);
	install_element(STRICT_ACCESS_NODE, &config_strict_access_level_cmd);

	install_element(CONFIG_NODE, &show_strict_access_config_cmd);
	install_element(STRICT_ACCESS_NODE, &show_strict_access_config_cmd);
}

