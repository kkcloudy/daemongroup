#ifndef __DCLI_STP_H__
#define __DCLI_STP_H__

#define STP_DISABLE 0xff
#define STP_HAVE_ENABLED 0xfe
#define STP_PORT_NOT_ENABLED 0xfd
#define STP_PORT_HAVE_ENABLED 0xfc
#define STP_PORT_NOT_LINK 0xfb

#define STP_DBUS_DEBUG(x) printf x
#define STP_DBUS_ERR(x) printf x

#define STP_DEBUG_FLAG_ALL       0xFF
#define STP_DEBUG_FLAG_DBG       0x1
#define STP_DEBUG_FLAG_WAR       0x2
#define STP_DEBUG_FLAG_ERR       0x4
#define STP_DEBUG_FLAG_EVT       0x8
#define STP_DEBUG_FLAG_PKT_REV   0x10
#define STP_DEBUG_FLAG_PKT_SED   0x20
#define STP_DEBUG_FLAG_PKT_ALL   0x30
#define STP_DEBUG_FLAG_PROTOCOL  0x40


#define STP_STR "Config Spanning-Tree Protocol\n"
#define MST_STR "Specify mst instance id <0-63>\n"

#define PRINTF_RSTP_NOT_ENABLED "RSTP hasn't enabled\n"
#define PRINTF_RSTP_HAVE_ENABLED  "RSTP is already enabled\n"
#define PRINTF_MSTP_NOT_ENABLED "MSTP hasn't enabled\n"
#define PRINTF_MSTP_HAVE_ENABLED  "MSTP is already enabled\n"
#define PRINTF_PORT_NOT_ENABLED "This port hasn't enabled\n"
#define PRINTF_PORT_HAVE_ENABLED  "This port is already enabled\n"
#define PRINTF_STP_Small_Bridge_Priority "The bridge priority is small\n"
#define PRINTF_STP_Large_Bridge_Priority "The bridge priority is large\n"
#define PRINTF_STP_Small_Max_Hops        "The max hops is small\n"
#define PRINTF_STP_Large_Max_Hops        "The max hops is large\n"
#define PRINTF_STP_Small_Hello_Time      "The hello time is small\n"
#define PRINTF_STP_Large_Hello_Time      "The hello time is large\n"
#define PRINTF_STP_Small_Max_Age         "The max age is small\n"
#define PRINTF_STP_Large_Max_Age         "The max age is large\n"
#define PRINTF_STP_Small_Forward_Delay   "The forward delay is small\n"
#define PRINTF_STP_Large_Forward_Delay   "The forward delay is large\n"
#define PRINTF_STP_Forward_Delay_And_Max_Age_Are_Inconsistent  "The forward delay and the max age should be content with: Max-Age<=2*(Forward-Delay-1)\n"
#define PRINTF_STP_Hello_Time_And_Max_Age_Are_Inconsistent     "The hello time and the max age should be content with:2*(hello-time+1)<=Max-Age\n"
#define PRINTF_STP_Hello_Time_And_Forward_Delay_Are_Inconsistent "The hello time and the forward delay should be contend with: 2*(hello-time +1)<=2*(forward-delay - 1)\n"


enum { 
  STP_OK = 0,                                     
  STP_ERROR,                                      
  STP_Cannot_Find_Vlan,      
  STP_Imlicite_Instance_Create_Failed,          
  STP_Small_Bridge_Priority,                    
  STP_Large_Bridge_Priority,                  
  STP_Small_Hello_Time,                      
  STP_Large_Hello_Time,                      
  STP_Small_Max_Age,                     
  STP_Large_Max_Age,                         
  STP_Small_Forward_Delay,                
  STP_Large_Forward_Delay,               
  STP_Small_Max_Hops,                   
  STP_Large_Max_Hops,                  
  STP_Forward_Delay_And_Max_Age_Are_Inconsistent,
  STP_Hello_Time_And_Max_Age_Are_Inconsistent, 
  STP_Hello_Time_And_Forward_Delay_Are_Inconsistent,
  STP_Vlan_Had_Not_Yet_Been_Created,           
  STP_Port_Is_Absent_In_The_Vlan,             
  STP_Big_len8023_Format,                     
  STP_Small_len8023_Format,               
  STP_len8023_Format_Gt_Len,            
  STP_Not_Proper_802_3_Packet,              
  STP_Invalid_Protocol,                    
  STP_Invalid_Version,                      
  STP_Had_Not_Yet_Been_Enabled_On_The_Vlan,   
  STP_Cannot_Create_Instance_For_Vlan,       
  STP_Cannot_Create_Instance_For_Port,      
  STP_Invalid_Bridge_Priority,           
  STP_There_Are_No_Ports,               
  STP_Cannot_Compute_Bridge_Prio,          
  STP_Another_Error,                    
  STP_Nothing_To_Do,                     
  STP_BRIDGE_NOTFOUND,                
  STP_CREATE_PORT_FAIL,                  
  STP_PORT_NOTFOUND,              
  STP_LAST_DUMMY                      
};

void dcli_stp_show_protocol_running_config
(
	int *state
) ;

#endif
