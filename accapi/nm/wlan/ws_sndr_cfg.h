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
* capture.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

/*这个文件通过一组宏定义决定相应的模块是否编译到最后的img中，实现模块在编译时的裁剪*/
/*这个模块最后可能有shell脚本来生成，在调试阶段，需要手动添加每个模块是否需要编译到程序中的定义。*/
/*有部分二级模块之间是相互关联的，这些关联的二级模块可以使用相同的宏定义。保证同时被编译到工程或者同时从工程中去除*/

//system 3
#define SNDR_USER_ITEM    1
#define SNDR_SYSTEM_ITEM  1
#define SNDR_SNMP_ITEM    1

//contrl 12
#define SNDR_PORT_ITEM    1
#define SNDR_FDB_ITEM     1
#define SNDR_VLAN_ITEM    1
#define SNDR_TRUNK_ITEM   1
#define SNDR_STP_ITEM     1
#define SNDR_ROUTE_ITEM   1 
#define SNDR_ACL_ITEM     1
#define SNDR_VRRP_ITEM    1
#define SNDR_QOS_ITEM     1
#define SNDR_DHCP_ITEM   1
#define SNDR_IGMP_ITEM    1
#define SNDR_TOOL_ITEM    1

//wlan 9
#define SNDR_SECURITY_ITEM  1
#define SNDR_WLAN_ITEM      1
#define SNDR_POINT_ITEM     1
#define SNDR_MONITOR_ITEM   1
#define SNDR_STATION_ITEM   1
#define SNDR_RF_ITEM        1
#define SNDR_ADVANCED_ITEM  1
#define SNDR_WLAN_QOS_ITEM  1
#define SNDR_EBR_ITEM       1
#define SNDR_WIDS_ITEM		1
  
//auth 1
#define SNDR_PORTAL_ITEM    1

//firewall 2
#define SNDR_FIREWALL_ITEM  1
#define SNDR_TRAFFIC_ITEM   1
#define SNDR_FACL_ITEM   1
 
//sys 1
#define SNDR_PERFORMANCE_ITEM 1

//help 1
#define SNDR_HELP_ITEM   1

