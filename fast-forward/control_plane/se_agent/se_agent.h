#ifndef SE_AGENT_H
#define SE_AGENT_H

#include "se_agent/se_agent_def.h"
#include "se_agent_acl.h"
#include "se_agent_user.h"
#include "se_agent_log.h"

#define FASTFWD_LOADED            0xff1
#define FASTFWD_NOT_LOADED        0xff2

#define SE_MAGIC_NUM         0x12345678

#define BASE_MAC_TABLE_NAME     "basemac_table"
#define PRODUCT_STATE_PATH      "/dbm/product_state"
#define BASE_MAC_PATH           "/devinfo/base_mac"
#define SLOT_COUNT_PATH         "/dbm/product/slotcount"


/*se_agent  error information ,the length of the string can not exceed ERR_INFO_SIZE(128)*/
#define FASTFWD_NO_RESPOND_STR      "%% Fast Forward is not responding"
#define SEND_FCCP_FAIL_STR          "%% Communication with the fastfwd module failed"
#define FASTFWD_NOT_LOADED_STR      "%% Fast forward module is not loaded"
#define RECV_NO_MEMORY_STR          "%% No memory receive from fast forward"
#define NO_THIS_RULE_STR            "%% No this rule"
#define READ_RULE_ERROR             "%% Read rule error"
#define NOT_HANDLE_FUNC_STR         "%% Not register command handle function"
#define USER_TBL_NO_USER_STR        "%% No this user"
#define FIND_USER_TLB_ERROR_STR     "%% Find user table error"
#define COMMAND_FAILED_STR          "%% Failed to execute the command"

#define PANEL_PORT_GROUP         0  /*group number of packets which received from the physical port */
#define FROM_LINUX_GROUP         2	/*group number of packets which linux send to SE*/
#define TO_LINUX_FCCP_GROUP      14 /*group number of fccp packets which SE send to linux*/
#define TO_LINUX_GROUP           15 /*group number of normal packets which SE send to linux*/
#define DEFAULT_AGENT_TIME       600

#define AGENT_TRUE  1
#define AGENT_FALSE 0
/*This define is the se_agent send fccp packet to se used*/
#define FCCP_SEND_GROUP         2
#define FCCP_WQE_TYPE           0x85
#define FCCP_DEFAULT_TAG        456


#define CMD_HANDLE_FUNC_NUM   100
typedef  void (*cmd_handle_func)(char *,struct sockaddr_tipc *,unsigned int);

typedef struct func_item_s
{
	char cmd_name[HAND_CMD_SIZE];
	struct func_item_s *next;
	cmd_handle_func  func;
	uint16_t valid_entries;
	uint8_t reserved[6];
}func_item_t;

extern int send_fccp(control_cmd_t  *fccp_cmd,unsigned char qos_data, unsigned char grp);
extern int fast_forward_module_load_check();
extern int se_agent_fccp_process(char *buf,unsigned int len, char back);
extern int32_t se_agent_pci_channel_init();
extern int se_agent_fccp_process_pcie(char *buf, unsigned int len, char need_response);

#ifdef SDK_VERSION_2_2
#define CVM_WQE_GET_LEN(work)   cvmx_wqe_get_len((work))
#define CVM_WQE_SET_LEN(work, v)   cvmx_wqe_set_len((work), (v))
#define CVM_WQE_GET_QOS(work)   cvmx_wqe_get_qos((work))
#define CVM_WQE_SET_QOS(work, v)   cvmx_wqe_set_qos((work), (v))
#define CVM_WQE_GET_PORT(work)   cvmx_wqe_get_port((work))
#define CVM_WQE_SET_PORT(work, v)   cvmx_wqe_set_port((work), (v))
#define CVM_WQE_GET_TAG(work)   ((work)->word1.tag)
#define CVM_WQE_SET_TAG(work, v)   ((work)->word1.tag = (v))
#define CVM_WQE_GET_TAG_TYPE(work)   ((work)->word1.tag_type)
#define CVM_WQE_SET_TAG_TYPE(work, v)   ((work)->word1.tag_type = (v))
#define CVM_WQE_GET_UNUSED(work)   cvmx_wqe_get_unused8((work))
#define CVM_WQE_SET_UNUSED(work, v)   cvmx_wqe_set_unused8((work), (v))
#define CVM_WQE_SET_GRP(work,v)    cvmx_wqe_set_grp((work),(v))
#define CVM_WQE_GET_GRP(work)  cvmx_wqe_get_grp((work))
#else
#define CVM_WQE_GET_LEN(work)   ((work)->len)
#define CVM_WQE_SET_LEN(work, v)   (work)->len = (v)
#define CVM_WQE_GET_QOS(work)   ((work)->qos)
#define CVM_WQE_SET_QOS(work, v)   ((work)->qos = (v))
#define CVM_WQE_GET_PORT(work)   ((work)->ipprt)
#define CVM_WQE_SET_PORT(work, v)   ((work)->ipprt = (v))
#define CVM_WQE_GET_TAG(work)   ((work)->tag)
#define CVM_WQE_SET_TAG(work, v)   ((work)->tag = (v))
#define CVM_WQE_GET_TAG_TYPE(work)   ((work)->tag_type)
#define CVM_WQE_SET_TAG_TYPE(work, v)   ((work)->tag_type = (v))
#define CVM_WQE_GET_UNUSED(work)   ((work)->unused)
#define CVM_WQE_SET_UNUSED(work, v)   ((work)->unused = (v))
#define CVM_WQE_SET_GRP(work,v)   ((work)->grp = (v))
#define CVM_WQE_GET_GRP(work)  ((work)->grp)

#endif


#endif

