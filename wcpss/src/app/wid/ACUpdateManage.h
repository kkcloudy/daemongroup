
#ifndef AC_UPDATE_MANAGE_H
#define AC_UPDATE_MANAGE_H

//when we have time, should move all other update wtp function to this file 

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h" 
#include "CWAC.h"
#include "ACDbus.h"
#include "ACDbus_handler.h"


CWBool insert_uptfail_wtp_list(int id);
CWBool delete_uptfail_wtp_list(unsigned int id);
CWBool find_in_uptfail_wtp_list(int id);
void destroy_uptfail_wtp_list();

void update_complete_check();

#endif

