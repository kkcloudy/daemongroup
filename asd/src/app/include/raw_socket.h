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
* raw_socket.h
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

#ifndef __RAW_SOCKET_H__
#define __RAW_SOCKET_H__
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include "auth.h"

//#define ETH_P_WAPI  0x88B4				/*WAI–≠“È∫≈*/
#define MAXMSG 2048

/*¥ÌŒÛ¬Î*/
#define RAW_SOCK_ERR_CREATE  -1
#define RAW_SOCK_ERR_INVALID_DEV_NAME -2
#define RAW_SOCK_ERR_GET_IFR  -3
#define RAW_SOCK_ERR_GET_MAC  -4
#define RAW_SOCK_ERR_SELECT   -5
#define RAW_SOCK_ERR_RECV     -6
#define RAW_SOCK_ERR_SEND     -7

int open_raw_socket(int eth_protocol_type);
int get_device_index_by_raw_socket(char* dev_name, int sock);
int get_device_mac_by_raw_socket(char* dev_name, int sock, u8 *mac_out);
int get_device_mtu_by_raw_socket(char* dev_name, int sock, u16 *mtu_out);
int handle_recverr(int sock);

int send_rs_data(const void *data, int len, struct ethhdr *eh, apdata_info *APData);

int recv_rs_data(void * data, int buflen, struct ethhdr *_eh, int sk);

#endif  

