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
* portal_ha.c
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

#ifndef _EAG_DBUS_H_
#define _EAG_DBUS_H_

#include <dbus/dbus.h>

#define MAX_DBUS_OBJPATH_LEN	128
#define MAX_DBUS_BUSNAME_LEN	128
#define MAX_DBUS_INTERFACE_LEN	128
#define MAX_DBUS_METHOD_LEN		128

typedef struct eag_dbus eag_dbus_t;

typedef DBusMessage *(*eag_dbus_method_func)(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data );

eag_dbus_t *
eag_dbus_new( const char *bus_name, 
		const char *obj_path );

int 
eag_dbus_reinit(eag_dbus_t *eag_dbus);

int 
eag_dbus_free( eag_dbus_t *eag_dbus );

DBusConnection *
eag_dbus_get_dbus_conn( const eag_dbus_t *eag_dbus );

int 
eag_dbus_register_method_name( 
			eag_dbus_t *eag_dbus, 
			const char *method_intf, 
			eag_dbus_method_func method_func,
			const char *method_name,
			void *method_param );

#define eag_dbus_register_method(eag_dbus,method_intf,method_func,method_param)\
			eag_dbus_register_method_name(eag_dbus,method_intf,method_func,#method_func,method_param)

void 
eag_dbus_dispach( eag_dbus_t *eag_dbus, 
			unsigned int block_usecond );

int
eag_dbus_send_signal( eag_dbus_t *eag_dbus,
								const char *obj_path,
								const char *interface_name,
								const char *signal_name, 
								int first_arg_type,
								va_list var_args  );								

#endif /*_EAG_DBUS_H_*/

