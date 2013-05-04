#ifndef _M_LOG_H_
#define _M_LOG_H_

void m_openlog (void);

void m_syslog (int priority, const char *message, ...);

void m_vsyslog(int priority, const char *message, va_list args);

#endif
