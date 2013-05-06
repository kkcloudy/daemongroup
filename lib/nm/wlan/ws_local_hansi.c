#include <syslog.h>
#include "hmd/hmdpub.h"
#include "ws_local_hansi.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "ws_init_dbus.h"



void free_broad_instance_info(struct Hmd_Board_Info_show **instanceHead) {

    if(NULL == instanceHead || NULL == *instanceHead)
        return ;
    
	void (*dcli_init_free_func)(struct Hmd_Board_Info_show *);
	if(NULL != ccgi_dl_handle) {
		dcli_init_free_func = dlsym(ccgi_dl_handle, "dcli_free_HmdBoardInfo");	
		if(NULL != dcli_init_free_func) {
			dcli_init_free_func(*instanceHead);
		}
	}
	
	*instanceHead = NULL;
	return ;
}

int show_broad_instance_info(DBusConnection *connection, struct Hmd_Board_Info_show **instanceHead) {
    if(NULL == connection || NULL == instanceHead) {
        syslog(LOG_WARNING, "show_broad_instance_info: input para error\n");
        return 0;
    }

    void *(*dcli_init_func) (DBusConnection *,
                    		int *, 
                    		int *);
    
    int ret = 0;
    int broad_num = 0;
    *instanceHead = NULL;
    
	if(NULL != ccgi_dl_handle) {
	
		dcli_init_func = dlsym(ccgi_dl_handle, "show_hmd_info_show");
		if(NULL != dcli_init_func) {
			*instanceHead = (*dcli_init_func)(connection,
                             				  &broad_num,
                             				  &ret);
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}

	if(0 == ret && instanceHead) 
        return 1;
	else if(HMD_DBUS_ID_NO_EXIST == ret) 
	    return -1;
	else {	    
        syslog(LOG_WARNING, "show_broad_instance_info: return 0\n");
	    return 0;    
    }	    
}


