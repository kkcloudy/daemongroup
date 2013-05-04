
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "board/board_define.h"
#include "ws_tcrule.h"
#include "ac_manage_def.h"

#include "ws_dbus_list_interface.h"
#include "ac_manage_public.h"

#include "ac_manage_tcrule.h"


#define TCSHELL_PATH "/opt/services/option/traffic_option"      /*defined in ws_tcrule.c!*/

#define TCRULE_MAX_NUM  256


static unsigned int service_status = 0;

static unsigned long tcRule_count = 0;
static TCRule *tcRuleRoot = NULL;

static int
tcrule_data_copy(TCRule *dest, TCRule *sour){
    if(NULL == dest || NULL == sour) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    dest->name = sour->name ? strdup(sour->name) : NULL;
    if(sour->name && NULL == dest->name) {
        return AC_MANAGE_MALLOC_ERROR;
    }

	dest->enable = sour->enable;
	dest->ruleIndex = sour->ruleIndex;

    dest->comment = sour->comment ? strdup(sour->comment) : NULL;
    if(sour->comment && NULL == dest->comment) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->interface = sour->interface ? strdup(sour->interface) : NULL;
    if(sour->interface && NULL == dest->interface) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->up_interface = sour->up_interface ? strdup(sour->up_interface) : NULL;
    if(sour->up_interface && NULL == dest->up_interface) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->protocol = sour->protocol ? strdup(sour->protocol) : NULL;
    if(sour->protocol && NULL == dest->protocol) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->p2p_detail = sour->p2p_detail ? strdup(sour->p2p_detail) : NULL;
    if(sour->p2p_detail && NULL == dest->p2p_detail) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->addrtype = sour->addrtype ? strdup(sour->addrtype) : NULL;
    if(sour->addrtype && NULL == dest->addrtype) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->addr_begin = sour->addr_begin ? strdup(sour->addr_begin) : NULL;
    if(sour->addr_begin && NULL == dest->addr_begin) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->addr_end = sour->addr_end ? strdup(sour->addr_end) : NULL;
    if(sour->addr_end && NULL == dest->addr_end) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->mode = sour->mode ? strdup(sour->mode) : NULL;
    if(sour->mode && NULL == dest->mode) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->uplink_speed = sour->uplink_speed ? strdup(sour->uplink_speed) : NULL;
    if(sour->uplink_speed && NULL == dest->uplink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->downlink_speed = sour->downlink_speed ? strdup(sour->downlink_speed) : NULL;
    if(sour->downlink_speed && NULL == dest->downlink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->useP2P = sour->useP2P;

    dest->p2p_uplink_speed = sour->p2p_uplink_speed ? strdup(sour->p2p_uplink_speed) : NULL;
    if(sour->p2p_uplink_speed && NULL == dest->p2p_uplink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    dest->p2p_downlink_speed = sour->p2p_downlink_speed ? strdup(sour->p2p_downlink_speed) : NULL;
    if(sour->p2p_downlink_speed && NULL == dest->p2p_downlink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->time_begin = sour->time_begin;    
    dest->time_end = sour->time_end;

    dest->limit_speed = sour->limit_speed ? strdup(sour->limit_speed) : NULL;
    if(sour->limit_speed && NULL == dest->limit_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    return AC_MANAGE_SUCCESS;
}

static int
insert_tcrule(TCRule **root, TCRule *newRule) {
    if(NULL == root || NULL == newRule) {
        syslog(LOG_WARNING, "insert_tcrule: input para error!\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    if(newRule->ruleIndex > TCRULE_MAX_NUM) {        
        syslog(LOG_WARNING, "insert_tcrule: The rule num is reach max!\n");
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }

    TCRule *node = NULL, *prior = NULL;
    for(node = *root, prior = node; node; prior = node, node = node->next) {
        if(node->ruleIndex == newRule->ruleIndex) {
            return AC_MANAGE_CONFIG_EXIST;
        }
        else if(node->ruleIndex > newRule->ruleIndex) {
        
            newRule->next = node;
            if(node == prior) {
                *root = newRule;
            }
            else {
                prior->next = newRule;
            }

            return AC_MANAGE_SUCCESS;
        }
    }

    newRule->next = NULL;
    if(NULL == prior) {
        *root = newRule;
    }
    else {
        prior->next = newRule;
    }

    return AC_MANAGE_SUCCESS;
}

static int
offset_tcrule(TCRule *root, struct tcrule_offset_s *offset) {
    if(NULL == offset || offset->ruleIndex > TCRULE_MAX_NUM) {
        syslog(LOG_WARNING, "offset_tcrule: input para error!\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }    
    
    TCRule *node = NULL;
    for(node = root; node; node = node->next) {
        if(node->ruleIndex == offset->ruleIndex) {
            memcpy(&node->offset, offset, sizeof(struct tcrule_offset_s));
            return AC_MANAGE_SUCCESS;
        }
        else if(node->ruleIndex > offset->ruleIndex) {
            break;
        }
    }
    
    return AC_MANAGE_CONFIG_NONEXIST;
}

static int
delete_tcrule(TCRule **root, unsigned int index) {
    if(NULL == root || 0 == index || index > TCRULE_MAX_NUM) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    TCRule *node = NULL, *prior = NULL;
    for(node = *root, prior = node; node; prior = node, node = node->next) {
        if(node->ruleIndex == index) {
        
            if(node == prior) {
                *root = node->next;
            }
            else {
                prior->next = node->next;
            }
            
            tcFreeRule(node);

            return AC_MANAGE_SUCCESS;
        }
        else if(node->ruleIndex > index) {
            syslog(LOG_INFO, "delete_tcrule, do not find the rule(index = %d)\n", index);
            break;
        }
    }

    return AC_MANAGE_CONFIG_NONEXIST;
}


int
manage_config_flow_control_service(unsigned int status) {

    if(status) {
        TCRule *root = tcRuleRoot;

        /*frist clear tc rule*/
        tc_doAllRules(NULL);
        remove(TCSHELL_PATH);
    
        if(root){
            tc_doAllRules(root);
        }
        service_status = 1;
    }
    else if(!status) {
        tc_doAllRules(NULL);
        remove(TCSHELL_PATH);
        service_status = 0;
    }

	return AC_MANAGE_SUCCESS;
}

int
manage_show_flow_control_service(void) {
    return service_status;
}


int
manage_add_tcrule(TCRule *tcRuleNew) {

    if(NULL == tcRuleNew) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int ret = AC_MANAGE_SUCCESS;
    ret = insert_tcrule(&tcRuleRoot, tcRuleNew);
    if(AC_MANAGE_SUCCESS == ret) {
        tcRule_count++;
        //tc_doAllRules(tcRuleRoot);
    }

    return ret;
}

int
manage_set_tcrule_offset(struct tcrule_offset_s *offset) {
    if(NULL == offset) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    return offset_tcrule(tcRuleRoot, offset);
}

int
manage_delete_tcrule(unsigned int index) {

    int ret = AC_MANAGE_SUCCESS;
    ret = delete_tcrule(&tcRuleRoot, index);
    if(AC_MANAGE_SUCCESS == ret) {
        tcRule_count--;
        //tc_doAllRules(tcRuleRoot);
    }

    return ret;
}

int
manage_show_tcrule(TCRule **rule_array, unsigned int *count) {
    if(NULL == rule_array || NULL == count)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
    *rule_array = NULL;
    *count = 0;
    
    if(tcRuleRoot && tcRule_count) {
        *rule_array = (TCRule *)calloc(tcRule_count, sizeof(TCRule));
        if(NULL == *rule_array) {
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        TCRule *node = NULL;
        for(i = 0, node = tcRuleRoot; i < tcRule_count && node; i++, node = node->next) {
            if(AC_MANAGE_SUCCESS != tcrule_data_copy(&((*rule_array)[i]), node)) {
                tcFreeArray(rule_array, tcRule_count);
                return AC_MANAGE_MALLOC_ERROR;
            }
        }
        
        *count = tcRule_count;
    }
    
    return AC_MANAGE_SUCCESS;
}

int
manage_show_tcrule_offset(struct tcrule_offset_s **offset_array, unsigned int *count) {
    if(NULL == offset_array || NULL == count)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
    *offset_array = NULL;
    *count = 0;
    
    if(tcRuleRoot && tcRule_count) {
        *offset_array = (struct tcrule_offset_s *)calloc(tcRule_count, sizeof(struct tcrule_offset_s));
        if(NULL == *offset_array) {
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        TCRule *node = NULL;
        for(i = 0, node = tcRuleRoot; i < tcRule_count && node; i++, node = node->next) {
            memcpy(&(*offset_array)[i], &node->offset, sizeof(struct tcrule_offset_s));
        }
        
        *count = tcRule_count;
    }
    
    return AC_MANAGE_SUCCESS;
}

int
manage_show_tcrule_running_config(struct running_config **configHead) {

    if(NULL == configHead) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *configHead = NULL;
    struct running_config *configEnd = NULL;
    
    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "manage_show_tcrule_running_config: get loal slot id error\n");
            manage_free_running_config(configHead);
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }
    
    if(tcRuleRoot && tcRule_count) {

        /*tc rule*/
        int i = 0; 
        TCRule *node = NULL;
        for(i = 0, node = tcRuleRoot; i < tcRule_count && node; i++, node = node->next) {
            if(node->enable && node->interface && node->up_interface && node->addrtype && node->addr_begin && node->uplink_speed && node->downlink_speed) {
                struct running_config *temp_config = manage_new_running_config();
                if(temp_config) {
                    if(0 == strcmp(node->addrtype, "address")) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " add tc %d-%d %s %s ip single %s %s %s", local_slotID, node->ruleIndex,
                                    node->up_interface, node->interface, node->addr_begin, node->uplink_speed, node->downlink_speed);

                        syslog(LOG_DEBUG, "address: manage_show_tcrule_running_config: temp_config->showstr = %s\n", temp_config->showStr);
                    }
                    else if(0 == strcmp(node->addrtype, "addrrange") && node->addr_end && node->mode) {
                        
                        char *strBWType = NULL;    
                        if (0 == strcmp(node->mode, "share")) {
                            strBWType = "shared";
                        }
                        else if (0 == strcmp(node->mode, "notshare")) {
                            strBWType = "non-shared";
                        }    
                        else {
                            MANAGE_FREE(temp_config);
                            continue;
                        }
                    
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " add tc %d-%d %s %s ip subnet %s/%s %s %s %s", local_slotID, node->ruleIndex,
                                    node->up_interface, node->interface, node->addr_begin, node->addr_end, strBWType, node->uplink_speed, node->downlink_speed);

                        syslog(LOG_DEBUG, "addrrange: manage_show_tcrule_running_config: temp_config->showstr = %s\n", temp_config->showStr);
                    }
                    else {
                        MANAGE_FREE(temp_config);
                        continue;
                    }
                    
                    manage_insert_running_config(configHead, &configEnd, temp_config);
                }   
            }
        }
    }
    
#if 0    
    /*tc service status*/
    if(service_status) {
        struct running_config *temp_config = manage_new_running_config();
        if(temp_config) {
            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " service enable");
        }
        manage_insert_running_config(configHead, &configEnd, temp_config);
    }
#endif
    
    return AC_MANAGE_SUCCESS;
}

