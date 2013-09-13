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
* Debug.h
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int debug;
#define DPrintf if (0) ; else printf
#define DPrint_string_array if(0); else print_string_array
#define DPrint_EC_KEY	if(0); else EC_KEY_print_fp_my
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDRESS "%02X%02X-%02X%02X-%02X%02X"//qiuchen add it for Henan Mobile 2013.02.20
/* For new format of syslog 2013-07-29 */
#define AUTELANID "@31656"
#define DS_STRUCTURE_ALL "[DS@31656 slot=\"%d\" inst=\"%d\"]"
#define DS_STRUCTURE "[DS@31656 slot=\"%d\"]"
#define LOG_MAC "mac=\"%02X:%02X:%02X:%02X:%02X:%02X\""
#define LOG_IP_V4 "ip=\"%lu.%lu.%lu.%lu\""
#define LOG_IP_STR "ip=\"%s\""
#define LOG_BSSID "bssid=\"%02X:%02X:%02X:%02X:%02X:%02X\""
#define LOG_IF "if=\"%s\""
#define LOG_NAME "name=\"%s\""
#define LOG_SSID "ssid=\"%s\""
#define LOG_SEC "security=\"%d\""
#define LOG_RADIO "radio=\"%d\""
#define LOG_RADIOS "radios=\"%d\""
#define LOG_CODE "reason code=\"%s%d\""
#define LOG_DESC "desc=\"%s\""
#define LOG_ROAM "ip=\"%lu.%lu.%lu.%lu\" bssid=\"%02X:%02X:%02X:%02X:%02X:%02X\" ap_mac=\"%02X:%02X:%02X:%02X:%02X:%02X\""
#define LOG_RDS "nas_id=\"%s\" nas_port_id=\"%s\""
#define LOG_ACT "tx=\"%lu\" rx=\"%lu\" tx_pkt=\"%lu\" rx_pkt=\"%lu\" online=\"%d\""
#define LOG_TYPE "type=\"%d\""
#define LOG_VLAN "vlan=\"%d\""
#define IPSTRINT(a) ((a & 0xff000000) >> 24),((a & 0xff0000) >> 16),((a & 0xff00) >> 8),(a & 0xff)

void PrintBuffer(const void* start, int offset);
char* SPrintBuffer(const void *start, int offset, int len );
void print_hex_string(void *_buf, int len);
void print_to_string(char* buf, int len);
void print_string(void *str,int len);
void print_string_array(char *name, void *_str,int len);

#endif //  #ifndef _MY_TRACEFUNC_H__

