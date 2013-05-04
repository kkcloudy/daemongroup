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

#ifndef _DRP_DBUS_H_
#define _DRP_DBUS_H_


#define DRP_DBUS_BUSNAME      "aw.drp"
#define DRP_DBUS_OBJPATH      "/aw/drp"
#define DRP_DBUS_INTERFACE    "aw.drp"

#define MAX_DBUS_OBJPATH_LEN	128
#define MAX_DBUS_BUSNAME_LEN	128
#define MAX_DBUS_INTERFACE_LEN	128
#define MAX_DBUS_METHOD_LEN		128

#define VRRP_DBUS_BUSNAME			"aw.vrrpcli"
#define VRRP_DBUS_OBJPATH 			"/aw/vrrp"
#define VRRP_DBUS_INTERFACE 		"aw.vrrp"
#define VRRP_DBUS_METHOD_SET_PORTAL_TRANSFER_STATE          "vrrp_set_portal_transfer_state"

typedef struct drp_dbus drp_dbus_t;
typedef DBusMessage *(*drp_dbus_method_func)(
		DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data );


drp_dbus_t *
drp_dbus_new( const char *bus_name, 
		const char *obj_path );

int 
drp_dbus_free( drp_dbus_t *drp_dbus );

DBusConnection *
drp_dbus_get_dbus_conn( const drp_dbus_t *drp_dbus );

int 
drp_dbus_register_method_name( 
		drp_dbus_t *drp_dbus, 
		const char *method_intf, 
		drp_dbus_method_func method_func,
		const char *method_name,
		void *method_param );

#define drp_dbus_register_method(drp_dbus,method_intf,method_func,method_param)\
	drp_dbus_register_method_name(drp_dbus,method_intf,method_func,#method_func,method_param)

void 
drp_dbus_dispach( drp_dbus_t *drp_dbus, 
		unsigned int block_usecond );



#endif /*_DRP_DBUS_H_*/

