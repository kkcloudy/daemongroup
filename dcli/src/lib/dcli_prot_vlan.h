#ifndef __DCLI_PROT_VLAN_H__
#define __DCLI_PROT_VLAN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "command.h"
#include "dcli_main.h"

#include "dbus/npd/npd_dbus_def.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nbm/npd_bmapi.h"
#include "sysdef/returncode.h"
#include "dcli_vlan.h"
#include "dcli_eth_port.h"

char * dcli_prot_vlan_error_info(unsigned int retCode);

void dcli_prot_vlan_element_init(void);


#endif
