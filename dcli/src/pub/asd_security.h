#ifdef _D_WCPSS_
#ifndef _ASD_SECURITY_H
#define _ASD_SECURITY_H

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"

#define SLOT_LLEGAL(slot_no)     ((slot_no)>0&&(slot_no)<=4)/*xm 08/08/27*/
#define PORT_LLEGAL(port_no)     ((port_no)>0&&(port_no)<=24)/*xm 08/08/27*/


#define SLOT_PORT_SPLIT_SLASH	'/'	/*xm 08/08/27*/
#define SLOT_PORT_SPLIT_DASH 	'-'	/*xm 08/08/27*/
#define SLOT_PORT_SPLIT_COMMA 	','	/*xm 08/08/27*/

#define MAX_NUM_OF_VLANID 256		/*xm 08/08/27*/
#define MAXINTERFACES 48			/*ht 08.12.02*/
#define MAXNAMSIZ 16				/*ht*/
#define MAX_VLAN_NUM 4094			/*ht add ->sz*/

typedef struct  {
       unsigned char slot;
       unsigned char port;
}SLOT_PORT_S;					/*xm 08/08/27*/

typedef enum {
		TEST_SLOT_STATE=1,
		TEST_SPLIT_STATE,
		TEST_PORT_STATE,
		TEST_COMMA_STATE,
		TEST_END_STATE,
		TEST_FAILURE_STATE,
		TEST_SUCESS_STATE
}PARSE_PORT_STATE;		/*xm 08/08/27  */

/*///////////////////////////////////////////////*/
/*sz20080825 */

typedef struct  {
       unsigned int vlanid;
       unsigned int stat;
	   
}VLAN_ENABLE;

typedef enum{
	check_vlanid_state=0,
	check_comma_state,
	check_fail_state,
	check_end_state,
	check_success_state
}vlan_list_state;

int _parse_port_list(char* ptr,int* count,SLOT_PORT_S spL[]);  /*xm 08/08/27*/
int parse_char_ID(char* str,unsigned char* ID);
int parse_int_ID(char* str,unsigned int* ID);
int Check_IP_Format(char* str);
int Check_Local_IP(char *check_ip);	/*ht add,08.12.02*/
void CheckRekeyMethod(char *type, unsigned char SecurityType);

void dcli_free_security(struct dcli_security *sec);
void dcli_free_security_list(struct dcli_security *sec);
struct dcli_security* show_radius_id(DBusConnection *dcli_dbus_connection,int index, unsigned char security_id, int localid, unsigned int *ret);
struct dcli_security* show_security_list(DBusConnection *dcli_dbus_connection,int index, unsigned char *security_num, int localid, unsigned int *ret);
struct dcli_security* show_security_id(DBusConnection *dcli_dbus_connection,int index, unsigned char security_id, int localid, unsigned int *ret);
struct dcli_security* show_security_wapi_info(DBusConnection *dcli_dbus_connection,int index, unsigned char security_id, int localid, unsigned int *ret);
struct dcli_security* show_wlan_security_wapi_info(DBusConnection *dcli_dbus_connection,int index, unsigned char wlan_id, int localid, unsigned int *ret);

void set_wapi_p12_cert_path(DBusConnection *dcli_dbus_connection, int index, unsigned char security_id, unsigned int type, 
	unsigned char *cert_path,  unsigned char *password, int localid, unsigned int *ret);




#endif
#endif

