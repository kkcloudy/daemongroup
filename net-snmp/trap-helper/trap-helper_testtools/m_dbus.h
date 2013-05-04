#ifndef _M_DBUS_H_
#define _M_DBUS_H_

#define M_DBUS_ERR -1
#define M_DBUS_OK 0
#define M_DBUS_OBJPATH "/test/dbus"
#define M_DBUS_INTERFACE "aw.trap"

int ac_sample_dbus_init(void);
int m_dbus_init(void);
int m_dbus_send_signal( const char *signal_name, int first_arg_type,...);

#endif
