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
* ACMultiHomedSocket.h
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

 
#ifndef __CAPWAP_CWMultiHomedSocket_HEADER__
#define __CAPWAP_CWMultiHomedSocket_HEADER__

#include "CWNetwork.h"
#include "wcpss/wid/WID.h"
/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
struct wlan_inf{
	unsigned char wlan_id;
	struct wlan_inf *wlan_next;

};


struct CWMultiHomedInterface{
	int  ipv6Flag;//0--ipv4 1--ipv6
	char ifname[IFI_NAME];
	CWNetworkLev4Address addr;
	CWNetworkLev4Address addrIPv4;
	CWNetworkLev4Address addrIPv6;
	CWSocket sock;
	enum {
		CW_PRIMARY,
		CW_BROADCAST_OR_ALIAS
	} kind;
	short systemIndex; // real interface index in the system
	short systemIndexbinding;
	int gIf_Index;
	char nas_id[128];
	LISTEN_FLAG lic_flag;
	struct CWMultiHomedInterface *if_next;
};


typedef struct {
	int count;
	struct CWMultiHomedInterface *interfaces;
} CWMultiHomedSocket;

/*_____________________________________________________*/
/*  *******************___MACRO___*******************  */

#define CW_COPY_MH_INTERFACE_PTR(int1, int2)		CW_COPY_NET_ADDR_PTR( &((int1)->addr), &((int2)->addr));	\
							CW_COPY_NET_ADDR_PTR( &((int1)->addrIPv4), &((int2)->addrIPv4));\
							(int1)->sock = (int2)->sock;					\
							(int1)->kind = (int2)->kind;	\
							(int1)->systemIndex = (int2)->systemIndex;\
							(int1)->systemIndexbinding = (int2)->systemIndexbinding;\
							memcpy((int1)->ifname,(int2)->ifname,IFI_NAME);


/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */

CWBool CWNetworkInitSocketServerMultiHomed(CWMultiHomedSocket *sockPtr, int port, char **multicastGroups, int multicastGroupsCount);
void CWNetworkCloseMultiHomedSocket(CWMultiHomedSocket *sockPtr);
CWBool CWNetworkUnsafeMultiHomed(CWMultiHomedSocket *sockPtr, void (*CWManageIncomingPacket) (CWSocket, char *, int, int,int, CWNetworkLev4Address*, char *), CWBool peekRead);
int CWNetworkCountInterfaceAddresses(CWMultiHomedSocket *sockPtr);
CWBool CWNetworkGetInterfaceAddresses(CWMultiHomedSocket *sockPtr, CWNetworkLev4Address **addressesPtr, struct sockaddr_in **IPv4AddressesPtr);
int CWNetworkCountInterfaceAddressesIpv4(CWMultiHomedSocket *sockPtr) ;
int CWNetworkCountInterfaceAddressesIpv6(CWMultiHomedSocket *sockPtr) ;
#endif
