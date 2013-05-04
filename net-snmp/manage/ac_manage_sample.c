#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "ac_manage_def.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_public.h"

#include "eag_errcode.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "ac_manage_sample_interface.h"

#include "ac_manage_sample.h"

void 
manage_free_radius_config(struct instance_radius_config **configHead) {
    if(NULL == configHead)
        return ;

    while(*configHead) {
        struct instance_radius_config *configNode = (*configHead)->next;
        MANAGE_FREE(*configHead);
        *configHead = configNode;
    }

    return ;
}

int
manage_get_master_radius_config(struct instance_radius_config **configHead, unsigned int *config_num) {
    syslog(LOG_DEBUG, "enter manage_get_master_radius_config\n");

    if(NULL == configHead || NULL == config_num) {
        syslog(LOG_WARNING, "manage_get_master_radius_config: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *configHead = NULL;
    *config_num = 0;

    int para_ret = AC_MANAGE_SUCCESS;
    instance_parameter *paraHead = NULL;
    para_ret = manage_get_master_instance_para(&paraHead);
    if(AC_MANAGE_SUCCESS == para_ret) {
        instance_parameter *paraNode = NULL;
        for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
            struct instance_radius_config *configNode = (struct instance_radius_config *)malloc(sizeof(struct instance_radius_config));
            if(NULL == configNode) {
                syslog(LOG_WARNING, "manage_get_master_radius_config: malloc %s %d-%d struct instance_radius_config failed!\n",
                                    paraNode->parameter.local_id ? "local-hansi" : "remote-hansi", paraNode->parameter.slot_id, paraNode->parameter.instance_id);
                continue;
            }
            
            memset(configNode, 0, sizeof(struct instance_radius_config));

            int ret;
            ret = eag_get_radius_conf(paraNode->connection, 
                                        paraNode->parameter.local_id, 
                                        paraNode->parameter.instance_id, 
                                        "",
                                        &configNode->radiusconf);

            if(0 == ret) {
                configNode->slot_id = paraNode->parameter.slot_id;
                configNode->local_id = paraNode->parameter.local_id;
                configNode->instance_id = paraNode->parameter.instance_id;

                configNode->next = *configHead;
                *configHead = configNode;
                (*config_num)++;
            }
            else {
                syslog(LOG_WARNING, "manage_get_master_radius_config: get %s %d-%d radius config failed!\n",
                                    paraNode->parameter.local_id ? "local-hansi" : "remote-hansi", paraNode->parameter.slot_id, paraNode->parameter.instance_id);
                MANAGE_FREE(configNode);
            }
        }
        
        manage_free_master_instance_para(&paraHead);        
    }
    else {
        syslog(LOG_WARNING, "manage_get_master_radius_config: manage_get_master_instance_para failed!\n");
        return para_ret;
    }

    syslog(LOG_DEBUG, "exit manage_get_master_radius_config\n");
    return AC_MANAGE_SUCCESS;
}

void 
manage_free_portal_config(struct instance_portal_config **configHead) {
    if(NULL == configHead)
        return ;

    while(*configHead) {
        struct instance_portal_config *configNode = (*configHead)->next;
        MANAGE_FREE(*configHead);
        *configHead = configNode;
    }

    return ;
}


int
manage_get_master_portal_config(struct instance_portal_config **configHead, unsigned int *config_num) {
    syslog(LOG_DEBUG, "enter manage_get_master_portal_config\n");

    if(NULL == configHead || NULL == config_num) {
        syslog(LOG_WARNING, "manage_get_master_portal_config: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *configHead = NULL;
    *config_num = 0;

    int para_ret = AC_MANAGE_SUCCESS;
    instance_parameter *paraHead = NULL;
    para_ret = manage_get_master_instance_para(&paraHead);
    if(AC_MANAGE_SUCCESS == para_ret) {
        instance_parameter *paraNode = NULL;
        for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
            struct instance_portal_config *configNode = (struct instance_portal_config *)malloc(sizeof(struct instance_portal_config));
            if(NULL == configNode) {
                syslog(LOG_WARNING, "manage_get_master_portal_config: malloc %s %d-%d struct instance_portal_config failed!\n",
                                    paraNode->parameter.local_id ? "local-hansi" : "remote-hansi", paraNode->parameter.slot_id, paraNode->parameter.instance_id);
                continue;
            }
            
            memset(configNode, 0, sizeof(struct instance_portal_config));

            int ret;
            ret = eag_get_portal_conf(paraNode->connection, 
                                        paraNode->parameter.local_id, 
                                        paraNode->parameter.instance_id, 
                                        &configNode->portalconf);

            if(0 == ret) {
                configNode->slot_id = paraNode->parameter.slot_id;
                configNode->local_id = paraNode->parameter.local_id;
                configNode->instance_id = paraNode->parameter.instance_id;

                configNode->next = *configHead;
                *configHead = configNode;
                (*config_num)++;
            }
            else {
                syslog(LOG_WARNING, "manage_get_master_portal_config: get %s %d-%d portal config failed!\n",
                                    paraNode->parameter.local_id ? "local-hansi" : "remote-hansi", paraNode->parameter.slot_id, paraNode->parameter.instance_id);
                MANAGE_FREE(configNode);
            }
        }

        manage_free_master_instance_para(&paraHead);
    }
    else {
        syslog(LOG_WARNING, "manage_get_master_portal_config: manage_get_master_instance_para failed!\n");
        return para_ret;
    }

    syslog(LOG_DEBUG, "exit manage_get_master_portal_config\n");
    return AC_MANAGE_SUCCESS;
}
