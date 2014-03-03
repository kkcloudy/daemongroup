#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <dbus/dbus.h>
#include <mcheck.h>

#include "facl_interface.h"

DBusConnection *dcli_dbus_connection = NULL;


int dcli_dbus_init(void) 
{
	DBusError dbus_error;

	dbus_error_init (&dbus_error);
	dcli_dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);
	if (dcli_dbus_connection == NULL) {
		printf ("dbus_bus_get(): %s", dbus_error.message);
		return FALSE;
	}

	dbus_bus_request_name(dcli_dbus_connection,"aw.new",0,&dbus_error);
	
	if (dbus_error_is_set(&dbus_error)) {
		printf("request name failed: %s", dbus_error.message);
		return FALSE;
	}
	return 0;
}

#if 0
int main(int argc, char *argv[])
{
	int ret = 0;
	uint32_t facl_tag = 0;
	char *facl_name = NULL;

	mtrace();
	if (argc != 3) {
		printf("used as :%s name tag\n", argv[0]);
		return 0;
	}
	facl_name = argv[1];
	facl_tag = atoi(argv[2]);
	
	dcli_dbus_init();

	ret = facl_create_policy(dcli_dbus_connection, facl_name, facl_tag);

	return ret;
}
#endif
int main(int argc, char *argv[])
{
	int ret = 0;
	uint32_t facl_tag = 15;
	uint32_t id = 6;
	int proto = 6;
	char *inif = NULL, *outif = NULL;
	char *srcip = NULL, *dstip = NULL;
	char *srcport = NULL, *dstport = NULL;

//	mtrace();

	inif = "eth1-3";
	outif = "any";
	srcip = "192.169.1.1";
	dstip = "100.1.1.1-100.1.1.100";
	srcport = "32";
	dstport = "23:43";
	
	dcli_dbus_init();

	ret = facl_interface_create_policy(dcli_dbus_connection, "aaa", facl_tag);

	ret = facl_interface_add_rule(dcli_dbus_connection, facl_tag, id, 0, inif, outif, srcip, dstip, proto, srcport, dstport);

	return ret;
}


