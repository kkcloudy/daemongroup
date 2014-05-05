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
* ACMultiHomeSocket.c
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

#include <time.h> 
#include "CWAC.h"
#include "wcpss/wid/WID.h"
#include "ACDbus_handler.h"
#include "ACNetlink.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

__inline__ void CWNetworkDeleteMHInterface(void *intPtr) {
	CW_FREE_OBJECT_WID(intPtr);
}

// multihomed sockets maps the system index for each interface to a array-like int index in range
// 0-(# of interfaces -1). This function returns the int index given the system index of an interface
// managed by the given multihomed socket.
int CWNetworkGetInterfaceIndexFromSystemIndex(CWMultiHomedSocket *sockPtr,int systemIndex, int sock) {
	int i, c;
	struct CWMultiHomedInterface *inf;
	if(sockPtr == NULL || systemIndex == -1) return -1;
	inf = sockPtr->interfaces;
	for(i = 0, c = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		if(inf->kind == CW_PRIMARY) {
			// each primary interface increments the int index
			if((inf->systemIndex == systemIndex)&&(inf->sock == sock)) return inf->gIf_Index;//zhanglei change c to inf->gIf_Index
			c++;
		}
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
	}
	
	return -1;
}

// check if the interface with system index systemIndex is already managed by the multihomed
// socket. If the answer is yes, returns informations on that interface, returns NULL otherwise
struct CWMultiHomedInterface *CWNetworkGetInterfaceAlreadyStored(CWList list, short systemIndex) {
	CWListElement *el;
	
	for(el = list; el != NULL; el = el->next) {
		if(((struct CWMultiHomedInterface*) (el->data))->systemIndex == systemIndex &&
			((struct CWMultiHomedInterface*) (el->data))->kind == CW_PRIMARY) return (struct CWMultiHomedInterface*) el->data;
	}
	
	return NULL;
}

// init multihomed socket. Will bind a socket for each interface + each broadcast address + the wildcard addres + each multicast address
// in multicastGroups.
/*
CWBool CWNetworkInitSocketServerMultiHomed(CWMultiHomedSocket *sockPtr, int port, char **multicastGroups, int multicastGroupsCount) {
	struct ifi_info	*ifi, *ifihead;
	CWNetworkLev4Address wildaddr;
    	int yes = 1;
	CWSocket sock;
	CWMultiHomedInterface *p;
	CWList interfaceList = CW_LIST_INIT;
	CWListElement *el = NULL;
	int i;
	if(sockPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	sockPtr->count = 0;
	
	// note: if get_ifi_info is called with AF_INET6 on an host that doesn't support IPv6, it'll simply act like if it was called with AF_INET
#ifdef CW_DEBUGGING
	// consider aliases as different interfaces (last arg of get_ifi_info is 1). Why? Just to increase the funny side of the thing
	// for each network interface
	for (ifihead = ifi = get_ifi_info((gNetworkPreferredFamily == CW_IPv6) ? AF_INET6 : AF_INET, 1); ifi != NULL; ifi = ifi->ifi_next) { 
#else
	for (ifihead = ifi = get_ifi_info((gNetworkPreferredFamily == CW_IPv6) ? AF_INET6 : AF_INET, 0); ifi != NULL; ifi = ifi->ifi_next) { // for each network interface
#endif
		// bind a unicast address
		if((sock = socket(ifi->ifi_addr->sa_family, SOCK_DGRAM, 0)) < 0) {
			free_ifi_info(ifihead);
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		// reuse address
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		
		// bind address
		sock_set_port_cw(ifi->ifi_addr, htons(port));
		
		if(bind(sock, (struct sockaddr*) ifi->ifi_addr, CWNetworkGetAddressSize((CWNetworkLev4Address*)ifi->ifi_addr)) < 0) {
			close(sock);
			CWUseSockNtop(ifi->ifi_addr,
				wid_syslog_debug_debug("failed %s", str);
			);
			
			continue;
			//CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		CWUseSockNtop(ifi->ifi_addr,
			wid_syslog_debug_debug("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
		);
		
		// store socket inside multihomed socket
		
		CW_CREATE_OBJECT_ERR(p, CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		p->sock = sock;
		if(CWNetworkGetInterfaceAlreadyStored(interfaceList, ifi->ifi_index) == NULL
					&& strncmp(ifi->ifi_name, "lo", 2)) { // don't consider loopback an interface
														  // (even if we accept packets from loopback)
			wid_syslog_debug_debug("Primary Address");
			p->kind = CW_PRIMARY;
		} else {
			p->kind = CW_BROADCAST_OR_ALIAS; // should be BROADCAST_OR_ALIAS_OR_MULTICAST_OR_LOOPBACK ;-)
			#ifdef CW_DEBUGGING
				if(!strncmp(ifi->ifi_name, "lo", 2)) {
					p->kind = CW_PRIMARY;
				}
			#endif
		}
		p->systemIndex = ifi->ifi_index;
		
		// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
		// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
		// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
		// garbage.
		p->addrIPv4.ss_family = AF_UNSPEC;
		CW_COPY_NET_ADDR_PTR(&(p->addr), ifi->ifi_addr);
		if(!CWAddElementToList(&interfaceList, p)) {
			return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
		}
		sockPtr->count++; // we add a socket to the multihomed socket
		
		
		if (ifi->ifi_flags & IFF_BROADCAST) { // try to bind broadcast address
			if((sock = socket(ifi->ifi_addr->sa_family, SOCK_DGRAM, 0)) < 0) {
				free_ifi_info(ifihead);
				CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
				CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
			
			// reuse address
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
			

			sock_set_port_cw(ifi->ifi_brdaddr, htons(port));
			
			if (bind(sock, (struct sockaddr*) ifi->ifi_brdaddr, CWNetworkGetAddressSize((CWNetworkLev4Address*)ifi->ifi_brdaddr)) < 0) {
				close(sock);
				if (errno == EADDRINUSE) {
					CWUseSockNtop(ifi->ifi_brdaddr,
					wid_syslog_debug_debug("EADDRINUSE: %s", str);
					);
					continue;
				} else {
					CWUseSockNtop(ifi->ifi_brdaddr,
					wid_syslog_debug_debug("failed %s", str);
					);
					continue;
					//CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
					//CWNetworkRaiseSystemError(CW_ERROR_CREATING);
				}
			}
			
			CWUseSockNtop(ifi->ifi_brdaddr,
				wid_syslog_debug_debug("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
			);
			
			// store socket inside multihomed socket
			
			CW_CREATE_OBJECT_ERR(p, CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			p->sock = sock;
			p->kind = CW_BROADCAST_OR_ALIAS;
			p->systemIndex = ifi->ifi_index;
			CW_COPY_NET_ADDR_PTR(&(p->addr), ifi->ifi_brdaddr);
			
			// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
			// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
			// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
			// garbage.
			p->addrIPv4.ss_family = AF_UNSPEC;
			if(!CWAddElementToList(&interfaceList, p)) {
				return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
			}
			sockPtr->count++; // we add a socket to the multihomed socket
		}

	}
	
	if(ifihead == NULL) { // get_ifi_info returned an error
		CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
		return CWErrorRaise(CW_ERROR_NEED_RESOURCE, "Error With get_ifi_info()");
	}
	
	free_ifi_info(ifihead);
	
#ifdef IPV6
	if(gNetworkPreferredFamily == CW_IPv6) { // we are an IPv6 server
		// store IPv4 addresses for our interfaces in the field "addrIPv4"
	
	#ifdef CW_DEBUGGING
		// consider aliases as different interfaces (last arg of get_ifi_info is 1). Why? Just to increase the funny side of the thing
		for (ifihead = ifi = get_ifi_info(AF_INET, 1); ifi != NULL; ifi = ifi->ifi_next) {
	#else
		for (ifihead = ifi = get_ifi_info(AF_INET, 0); ifi != NULL; ifi = ifi->ifi_next) {
	#endif
			CWMultiHomedInterface *s = CWNetworkGetInterfaceAlreadyStored(interfaceList, ifi->ifi_index);
			
			if(s == NULL || s->kind != CW_PRIMARY || s->addrIPv4.ss_family != AF_UNSPEC || ifi->ifi_addr->sa_family != AF_INET) continue;
			
			CW_COPY_NET_ADDR_PTR(&(s->addrIPv4), ifi->ifi_addr);
			
			CWUseSockNtop(&(s->addrIPv4),
			wid_syslog_debug_debug("IPv4 address %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
			);
		}
		
		if(ifihead == NULL) { // get_ifi_info returned an error
			CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
			return CWErrorRaise(CW_ERROR_NEED_RESOURCE, "Error with get_ifi_info()");
		}
	
		free_ifi_info(ifihead);
	}
#endif
	
	
	// bind wildcard address
#ifdef	IPV6
	if (gNetworkPreferredFamily == CW_IPv6) {
		if((sock = socket(AF_INET6,SOCK_DGRAM,0)) < 0) {
			goto fail;
		}
	} else
#endif
	{
		if((sock = socket(AF_INET,SOCK_DGRAM, 0)) < 0) goto fail;
	}
	
	goto success;
	
fail:
	CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
	CWNetworkRaiseSystemError(CW_ERROR_CREATING); // this wil return
	// not reached
	
success:
	// reuse address
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	CW_ZERO_MEMORY(&wildaddr, sizeof(wildaddr));
	
#ifdef	IPV6
	if (gNetworkPreferredFamily == CW_IPv6) {
		// fill wildaddr considering it an IPv6 addr
		struct sockaddr_in6 *a = (struct sockaddr_in6 *) &wildaddr;
		a->sin6_family = AF_INET6;
		a->sin6_addr = in6addr_any;
		a->sin6_port = htons(port);
	} else
#endif
	{
		// fill wildaddr considering it an IPv4 addr
		struct sockaddr_in *a = (struct sockaddr_in *) &wildaddr;
		a->sin_family = AF_INET;
		a->sin_addr.s_addr = htonl(INADDR_ANY);
		a->sin_port = htons(port);
	}
	
	if(bind(sock, (struct sockaddr*) &wildaddr, CWNetworkGetAddressSize(&wildaddr)) < 0) {
		close(sock);
		CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
		CWNetworkRaiseSystemError(CW_ERROR_CREATING);
	}
	
	CWUseSockNtop(&wildaddr,
		CWLog("bound %s", str);
	);
	
	CW_CREATE_OBJECT_ERR(p, CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	p->sock = sock;
	p->kind = CW_BROADCAST_OR_ALIAS;
	p->systemIndex = -1; // make sure this can't be confused with an interface
	
	// addrIPv4 field for the wildcard address cause it is garbage in both cases (IPv4 + IPv6)
	p->addrIPv4.ss_family = AF_UNSPEC;

	CW_COPY_NET_ADDR_PTR(&(p->addr), &wildaddr);
	if(!CWAddElementToList(&interfaceList, p)) {
		return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
	}
	sockPtr->count++;
	
	
	// bind multicast addresses
	
	for(i = 0; i < multicastGroupsCount; i++) {
		struct addrinfo hints, *res, *ressave;
		char serviceName[5];
		CWSocket sock;
		
		CW_ZERO_MEMORY(&hints, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		
		snprintf(serviceName, 5, "%d", CW_CONTROL_PORT); // endianness will be handled by getaddrinfo
		
		wid_syslog_debug_debug("Joining Multicast Group: %s...", multicastGroups[i]);
		
		if (getaddrinfo(multicastGroups[i], serviceName, &hints, &res) != 0 ) {
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		ressave = res;
		
		do {
			if((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
				continue; // try next address
			}
			
			// reuse address
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	
			if(bind(sock, res->ai_addr, res->ai_addrlen) == 0) break; // success
			
			close(sock); // failure
		} while ( (res = res->ai_next) != NULL);
		
		if(res == NULL) { // error on last iteration
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		if(mcast_join(sock, res->ai_addr, res->ai_addrlen, NULL, 0) != 0) {
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		CWUseSockNtop((res->ai_addr),
			wid_syslog_debug_debug("Joined Multicast Group: %s", str);
		);
		
		CW_CREATE_OBJECT_ERR(p, CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		p->sock = sock;
		p->kind = CW_BROADCAST_OR_ALIAS;
		p->systemIndex = -1;
		
		
		p->addrIPv4.ss_family = AF_UNSPEC;
		
		CW_COPY_NET_ADDR_PTR(&(p->addr), res->ai_addr);
		if(!CWAddElementToList(&interfaceList, p)) {
			return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
		}
		sockPtr->count++; // we add a socket to the multihomed socket
		
		freeaddrinfo(ressave);
	}
	
	
	// Lists are fun when you don't know how many sockets will not give an error on creating/binding, but now that
	// we know the exact number we convert it into an array. The "interfaces" field of CWMultiHomedSocket is
	// actually an array
	
	CW_CREATE_ARRAY_ERR((sockPtr->interfaces), sockPtr->count, CWMultiHomedInterface,
					return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	// create array from list
	for(el = interfaceList, i = 0; el != NULL; el = el->next, i++) {
		CW_COPY_MH_INTERFACE_PTR(&((sockPtr->interfaces)[i]), ((CWMultiHomedInterface*)(el->data)));
	}
	
	// delete the list
	CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
	
	return CW_TRUE;
}
*/
CWBool CWNetworkInitSocketServerMultiHomed(CWMultiHomedSocket *sockPtr, int port, char **multicastGroups, int multicastGroupsCount) {
		CWNetworkLev4Address wildaddr;
			int yes = 1;
		CWSocket sock;
		struct CWMultiHomedInterface *p;
		CWList interfaceList = CW_LIST_INIT;
		CWListElement *el = NULL;
		int i;
		if(sockPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		
	//	sockPtr->count = 0;//zhanglei mv it out
		if(vrrid == 0){//wuwl add for listen Specific interface and  listen locahost only in config node,will not listen localhost in hansi
			// note: if get_ifi_info is called with AF_INET6 on an host that doesn't support IPv6, it'll simply act like if it was called with AF_INET
			if((sock = socket(AF_INET,SOCK_DGRAM, 0)) < 0) goto fail;
			
			goto success;
			
		fail:
			CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
			CWNetworkRaiseSystemError(CW_ERROR_CREATING); // this wil return
			// not reached
			
		success:
			// reuse address
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		
			CW_ZERO_MEMORY(&wildaddr, sizeof(wildaddr));
			
#ifdef	IPV6
			if (gNetworkPreferredFamily == CW_IPv6) {
				// fill wildaddr considering it an IPv6 addr
				struct sockaddr_in6 *a = (struct sockaddr_in6 *) &wildaddr;
				a->sin6_family = AF_INET6;
				a->sin6_addr = in6addr_any;
				a->sin6_port = htons(port);
			} else
#endif
			{
				// fill wildaddr considering it an IPv4 addr
				struct sockaddr_in *a = (struct sockaddr_in *) &wildaddr;
				a->sin_family = AF_INET;
				a->sin_addr.s_addr = htonl(INADDR_ANY);
				a->sin_port = htons(port);
			}
			
			if(bind(sock, (struct sockaddr*) &wildaddr, CWNetworkGetAddressSize(&wildaddr)) < 0) {
				close(sock);
				CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
				CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
		
			CWUseSockNtop(&wildaddr,
				wid_syslog_info("bound %s", str);
			);
			
			CW_CREATE_OBJECT_ERR_WID(p, struct CWMultiHomedInterface, close(sock); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(p->ifname, 0, IFI_NAME);
			if(p->ifname != NULL){
				memcpy(p->ifname,"LocalHost",9);
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			p->sock = sock;
			p->kind = CW_BROADCAST_OR_ALIAS;
			p->systemIndex = -1; // make sure this can't be confused with an interface
			p->systemIndexbinding = -1;
			
			// addrIPv4 field for the wildcard address cause it is garbage in both cases (IPv4 + IPv6)
			p->addrIPv4.ss_family = AF_UNSPEC;
		
			CW_COPY_NET_ADDR_PTR(&(p->addr), &wildaddr);
			if(!CWAddElementToList(&interfaceList, p)) {
				return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
			}
			sockPtr->count++;
		}
		
		// bind multicast addresses
	
		for(i = 0; i < multicastGroupsCount; i++) {
			struct addrinfo hints, *res, *ressave;
			char serviceName[5];
			CWSocket sock;
			
			CW_ZERO_MEMORY(&hints, sizeof(struct addrinfo));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;
			
			//snprintf(serviceName, 5, "%d", CW_CONTROL_PORT); // endianness will be handled by getaddrinfo
			snprintf(serviceName, 5, "%d", port);//zhanglei change
			wid_syslog_info("Joining Multicast Group: %s...", multicastGroups[i]);
			
			if (getaddrinfo(multicastGroups[i], serviceName, &hints, &res) != 0 ) {
				CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
			
			ressave = res;
			
			do {
				if((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
					continue; // try next address
				}
				
				// reuse address
				setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		
				if(bind(sock, res->ai_addr, res->ai_addrlen) == 0) break; // success
				
				close(sock); // failure
			} while ( (res = res->ai_next) != NULL);
			
			if(res == NULL) { // error on last iteration
				CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
			
			if(mcast_join(sock, res->ai_addr, res->ai_addrlen, NULL, 0) != 0) {
				close(sock);
				CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
			
			CWUseSockNtop((res->ai_addr),
				wid_syslog_info("Joined Multicast Group: %s", str);
			);
			
			CW_CREATE_OBJECT_ERR_WID(p, struct CWMultiHomedInterface, close(sock); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(p->ifname, 0, IFI_NAME);			
			if(p->ifname != NULL){
				memcpy(p->ifname,"LocalHost",9);
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			p->sock = sock;
			p->kind = CW_BROADCAST_OR_ALIAS;
			p->systemIndex = -1;
			p->systemIndexbinding = -1;
			
			
			p->addrIPv4.ss_family = AF_UNSPEC;
			
			CW_COPY_NET_ADDR_PTR(&(p->addr), res->ai_addr);
			if(!CWAddElementToList(&interfaceList, p)) {
				return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
			}
			sockPtr->count++; // we add a socket to the multihomed socket
			
			freeaddrinfo(ressave);
		}
		
		
		// Lists are fun when you don't know how many sockets will not give an error on creating/binding, but now that
		// we know the exact number we convert it into an array. The "interfaces" field of CWMultiHomedSocket is
		// actually an array
		/*
		CW_CREATE_ARRAY_ERR((sockPtr->interfaces), sockPtr->count, CWMultiHomedInterface,
						return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		*/
		// create array from list
		
		for(el = interfaceList, i = 0; el != NULL; el = el->next, i++) {
			struct CWMultiHomedInterface *inf;
			
			
			inf = (struct CWMultiHomedInterface *)WID_MALLOC(sizeof(struct CWMultiHomedInterface));
			if (NULL == inf)
			{
				wid_syslog_err("func %s,line %d,malloc fail\n",__func__,__LINE__);
				continue;				
			}
			
	//		inf->wlaninfo = (struct wlan_inf*)malloc(sizeof(struct wlan_inf));
			

	//		inf->wlaninfo->wlan_id = WlanID;
	//		inf->wlaninfo->wlan_next = NULL;
			

			CW_COPY_MH_INTERFACE_PTR(inf, ((struct CWMultiHomedInterface*)(el->data)));
			inf->gIf_Index = -1;
			if(sockPtr->interfaces == NULL)
			{	
				sockPtr->interfaces = inf;
				
				sockPtr->interfaces->if_next = NULL;
				
	//			printf("wlanid:%d, index:%d, kind:%d\n",sockPtr->interfaces->wlaninfo->wlan_id,sockPtr->interfaces->systemIndex,sockPtr->interfaces->kind);
				
			}else{
				
				inf->if_next = sockPtr->interfaces;
				sockPtr->interfaces = inf;
				
	//			printf("wlanid:%d, index:%d, kind:%d\n",inf->wlaninfo->wlan_id,inf->systemIndex,inf->kind);
				
			}
			
	//		printf("i: %d\n",i);

		}
		

		// delete the list
		CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
		
		Check_gACSokcet_Poll(sockPtr);
		return CW_TRUE;
	}



void CWNetworkCloseMultiHomedSocket(CWMultiHomedSocket *sockPtr) {
	int i = 0;
	struct CWMultiHomedInterface *inf;
	if(sockPtr == NULL || sockPtr->interfaces == NULL) return;
	inf = sockPtr->interfaces;
	for(i = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		close(inf->sock);
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
	}
	CW_FREE_OBJECT_WID(sockPtr->interfaces);
	sockPtr->count = 0;
}

#if 0
// blocks until one ore more interfaces are ready to read something. When there is at least one packet pending, call CWManageIncomingPacket() for each
// pending packet, then return.
CWBool CWNetworkUnsafeMultiHomed(CWMultiHomedSocket *sockPtr, void (*CWManageIncomingPacket) (CWSocket, char *, int, int,int, CWNetworkLev4Address*,char *), CWBool peekRead) {
	fd_set fset;
	struct timeval timeout;
	int max = 0, i;
	CWNetworkLev4Address addr;
	struct CWMultiHomedInterface *inf;
	int flags = ((peekRead != CW_FALSE) ? MSG_PEEK : 0);
	//int widloglevel = WID_LOG_ALL;
	char buf[CW_BUFFER_SIZE];
	
	if(sockPtr == NULL || CWManageIncomingPacket == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	FD_ZERO(&fset);
	inf = sockPtr->interfaces;
	// select() on all the sockets
	for(i = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		FD_SET(inf->sock, &fset);
		if(inf->sock > max) max = inf->sock;
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
	}
	FD_SET(netlink.sock,&fset);
	if(netlink.sock > max)
		max = netlink.sock;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	
	while(select(max+1, &fset, NULL, NULL, &timeout) < 0) {
		if(errno != EINTR) {
			CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
		}
	}
	
	// calls CWManageIncomingPacket() for each interface that has an incoming packet
	
	if(FD_ISSET(netlink.sock, &fset)) 
	{
		netlink_parse_info(netlink_information_fetch, &netlink);
	}
	inf = sockPtr->interfaces;
	for(i = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		if(FD_ISSET(inf->sock, &fset)) {
			int readBytes;
			if(WID_WATCH_DOG_OPEN == 1){
				if((inf->gIf_Index != -1)&&(inf->gIf_Index < gMaxInterfacesCount)){
					if(gInterfaces[inf->gIf_Index].tcpdumpflag == 1){
						gInterfaces[inf->gIf_Index].datacount++;
					}
				}else if(inf->gIf_Index == -1){
					G_LocalHost_num++;
				}
			}
			/*##			
				
				CWUseSockNtop(&(sockPtr->interfaces[i].addr),
				wid_syslog_debug_debug("Ready on %s", str);
			);*/
			
			CW_ZERO_MEMORY(buf, CW_BUFFER_SIZE);
			
			// message
			if(!CWErr(CWNetworkReceiveUnsafe(inf->sock, buf, CW_BUFFER_SIZE-1, flags, &addr, &readBytes))) {
				//sleep(1);			
				if(inf->if_next == NULL)
					break;
				inf = inf->if_next;
				continue;
			}
		if(is_secondary == 1){
			return CW_FALSE;
		}
		if(readBytes > 4096){	
			CWUseSockNtop(&addr,				
				wid_syslog_err("%s  %s,readBytes = %d.readBytes more than 2048.\n",str,__func__,readBytes);
			);
 			return CW_FALSE;
		}
			#if 1
			int temp_ret = CWNetworkGetInterfaceIndexFromSystemIndex(sockPtr, inf->systemIndex,inf->sock);
			if (temp_ret == -1) {
				wid_syslog_err("%s %d temp_ret=-1\n", __FUNCTION__, __LINE__);
				break;
			}
			CWManageIncomingPacket(inf->sock, buf, readBytes, temp_ret,inf->systemIndex, &addr, inf->ifname);
			#else
			CWManageIncomingPacket(inf->sock, buf, readBytes, CWNetworkGetInterfaceIndexFromSystemIndex(sockPtr, inf->systemIndex,inf->sock),inf->systemIndex, &addr, inf->ifname);
			#endif
		}
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
//		else {wid_syslog_debug_debug("~~~~~~~Non Ready on....~~~~~~");}
	}

	
	return CW_TRUE;
}
#endif
// blocks until one ore more interfaces are ready to read something. When there is at least one packet pending, call CWManageIncomingPacket() for each
// pending packet, then return.
//poll for select error socket repair
CWBool CWNetworkUnsafeMultiHomed(CWMultiHomedSocket *sockPtr, void (*CWManageIncomingPacket) (CWSocket, char *, int, int,int, CWNetworkLev4Address*,char *), CWBool peekRead) {
	int  i;
	int t = 0;

	CWNetworkLev4Address addr;
	struct CWMultiHomedInterface *inf;
	nfds_t nfds = 0;
	int flags = ((peekRead != CW_FALSE) ? MSG_PEEK : 0);

	char buf[CW_BUFFER_SIZE];
	if(sockPtr == NULL || CWManageIncomingPacket == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	nfds = sockPtr->count +1;
	inf = sockPtr->interfaces;


	sockPtr->pfd[nfds-1].fd=netlink.sock;
	sockPtr->pfd[nfds-1].events = POLLIN|POLLPRI;
	t = 2000;// 2000 ms 2s
	if(poll(sockPtr->pfd,nfds,t)<0){
		wid_syslog_info("%s,%d,errno = %d,error:%s",__func__,__LINE__,errno,strerror(errno));
	}

	if(((sockPtr->pfd[nfds-1].revents&POLLIN) == POLLIN)||((sockPtr->pfd[nfds-1].revents&POLLPRI) == POLLPRI))
	{
		netlink_parse_info(netlink_information_fetch, &netlink);
		return CW_TRUE;

	}
	else if(((sockPtr->pfd[nfds-1].revents&POLLERR) == POLLERR)||((sockPtr->pfd[nfds-1].revents&POLLNVAL) == POLLNVAL))
	{
		wid_syslog_info("%s,%d,I trust in netlink but it error\n",__func__,__LINE__);
		return CW_FALSE;
	}
	else{

		inf = sockPtr->interfaces;
		for(i = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
			if(((sockPtr->pfd[i].revents&POLLIN) == POLLIN)||((sockPtr->pfd[i].revents&POLLPRI) == POLLPRI)){
				int readBytes;

				if(WID_WATCH_DOG_OPEN == 1){
					if((inf->gIf_Index != -1)&&(inf->gIf_Index < gMaxInterfacesCount)){
						if(gInterfaces[inf->gIf_Index].tcpdumpflag == 1){
							gInterfaces[inf->gIf_Index].datacount++;
						}
					}else if(inf->gIf_Index == -1){
						G_LocalHost_num++;
					}
				}
				/*##			
					
					CWUseSockNtop(&(sockPtr->interfaces[i].addr),
					wid_syslog_debug_debug("Ready on %s", str);
				);*/
				
				CW_ZERO_MEMORY(buf, CW_BUFFER_SIZE);
				
				// message
				if(!CWErr(CWNetworkReceiveUnsafe(inf->sock, buf, CW_BUFFER_SIZE-1, flags, &addr, &readBytes))) {
					//sleep(1);			
					if(inf->if_next == NULL)
						break;
					inf = inf->if_next;
					continue;
				}
				if(is_secondary == 1){
					return CW_FALSE;
				}
				if(readBytes > 4096){	
					CWUseSockNtop(&addr,				
						wid_syslog_err("%s  %s,readBytes = %d.readBytes more than 2048.\n",str,__func__,readBytes);
					);
		 			return CW_FALSE;
				}
				CWManageIncomingPacket(inf->sock, buf, readBytes,
						CWNetworkGetInterfaceIndexFromSystemIndex(sockPtr, inf->systemIndex,inf->sock),inf->systemIndex, &addr, inf->ifname);
			}
			else if(((sockPtr->pfd[i].revents&POLLERR) == POLLERR)||((sockPtr->pfd[i].revents&POLLNVAL) == POLLNVAL)){
				wid_syslog_info("%s,%d wid will repair the error socket\n",__func__,__LINE__);
				if(Repair_WID_Listening_Socket(inf)<0){
					wid_syslog_info("%s,%d wid repair socket error\n",__func__,__LINE__);
				}
				else{
					wid_syslog_info("%s,%d wid repair socket successfully\n",__func__,__LINE__);
				}
			}
			inf = inf->if_next;
	//		else {wid_syslog_debug_debug("~~~~~~~Non Ready on....~~~~~~");}
		}
	}
	return CW_TRUE;
}
// divide gInterfaceCount into ipv4 and ipv6
int CWNetworkCountInterfaceAddressesIpv4(CWMultiHomedSocket *sockPtr) {
	int count = 0;
	int i;
	struct CWMultiHomedInterface *inf;
	if(sockPtr == NULL) return 0;
	inf = sockPtr->interfaces;
    if(inf !=NULL){
	    for(i = 0; i < sockPtr->count; i++) {
				if((inf->kind == CW_PRIMARY)&&(inf->ipv6Flag == 0)) 
					count++;  //count ipv4
				if(inf->if_next == NULL)
					break;
				inf = inf->if_next;
		}
 	}
	return count;
}
int CWNetworkCountInterfaceAddressesIpv6(CWMultiHomedSocket *sockPtr) {
	int count = 0;
	int i;
	struct CWMultiHomedInterface *inf;
	if(sockPtr == NULL) return 0;
	inf = sockPtr->interfaces;
	if(inf != NULL){
	 for(i = 0; i < sockPtr->count; i++) {
			if((inf->kind == CW_PRIMARY)&&(inf->ipv6Flag == 1)) 
				  count++;//count ipv6
			if(inf->if_next == NULL)
				break;
			inf = inf->if_next;
		}
    }
	return count;
}


// count distinct interfaces managed by the multihomed socket
int CWNetworkCountInterfaceAddresses(CWMultiHomedSocket *sockPtr) {
	int count = 0;
	int i;
	struct CWMultiHomedInterface *inf;
	if(sockPtr == NULL) return 0;
	inf = sockPtr->interfaces;
//	printf("sockPtr->count:%d\n",sockPtr->count);
	for(i = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		if(inf){
			if(inf->kind == CW_PRIMARY) count++;
			if(inf->if_next == NULL)
				break;
			inf = inf->if_next;
		}
	}
//	printf("count:%d\n",count);
	return count;
}

// get the addresses of each distinct interface managed by the multihomed socket. If we are an IPv6 server element with index i of addressesPtr contains
// the IPv6 address of the interface at index i (our mapped index, not system index) and the element at index i of IPv4AddressesPtr contains the IPv4
// equivalent address for the interface at index i. If we are an IPv4 server, addressesPtr are the IPv4 addresses and IPv4AddressesPtr is garbage.
CWBool CWNetworkGetInterfaceAddresses(CWMultiHomedSocket *sockPtr, CWNetworkLev4Address **addressesPtr, struct sockaddr_in **IPv4AddressesPtr) {
	int i, j;
	
	struct CWMultiHomedInterface *inf;
	if(sockPtr == NULL || addressesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CW_CREATE_ARRAY_ERR(*addressesPtr, CWNetworkCountInterfaceAddresses(sockPtr), CWNetworkLev4Address,
						return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	if(IPv4AddressesPtr != NULL && gNetworkPreferredFamily == CW_IPv6) {
		CW_CREATE_ARRAY_ERR(*IPv4AddressesPtr, CWNetworkCountInterfaceAddresses(sockPtr), struct sockaddr_in,
						return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	}
	
	inf = sockPtr->interfaces;
	for(i = 0, j = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		if(inf->kind == CW_PRIMARY) {
			CW_COPY_NET_ADDR_PTR(&((*addressesPtr)[j]), ((CWNetworkLev4Address*)&(inf->addr)));
			if(IPv4AddressesPtr != NULL && gNetworkPreferredFamily == CW_IPv6) {
				CW_COPY_NET_ADDR_PTR(&((*IPv4AddressesPtr)[j]), ((CWNetworkLev4Address*)&(inf->addrIPv4)));
			}
			j++;
		}
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
		
	}
	
	return CW_TRUE;
}



