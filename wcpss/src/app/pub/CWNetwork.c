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
* CWNetwork.c
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

 
#include "CWCommon.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWNetworkLev3Service gNetworkPreferredFamily = CW_IPv6;

__inline__ int CWNetworkGetAddressSize(CWNetworkLev4Address *addrPtr) {
	// assume address is valid
	
	switch ( ((struct sockaddr*)(addrPtr))->sa_family ) {
		
	#ifdef	IPV6
	// IPv6 is defined in Stevens' library
		case AF_INET6:
			return sizeof(struct sockaddr_in6);
			break;
	#endif
		case AF_INET:
		default:
			return sizeof(struct sockaddr_in);
	}
}

// send buf on an unconnected UDP socket. Unsafe means that we don't use DTLS
CWBool CWNetworkSendUnsafeUnconnected(CWSocket sock, CWNetworkLev4Address *addrPtr, const char *buf, int len) 
{
	if(buf == NULL || addrPtr == NULL) 
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CWUseSockNtop(addrPtr, CWDebugLog(str););

	while(sendto(sock, buf, len, 0, (struct sockaddr*)addrPtr, CWNetworkGetAddressSize(addrPtr)) < 0) {
		if(errno == EINTR){
			wid_syslog_err("<sendto error>%s,LINE=%d\n",__func__,__LINE__);
			continue;
		}
		CWNetworkRaiseSystemError(CW_ERROR_SENDING);
	}
	
	return CW_TRUE;
}

// send buf on a "connected" UDP socket. Unsafe means that we don't use DTLS
CWBool CWNetworkSendUnsafeConnected(CWSocket sock, const char *buf, int len) 
{
	if(buf == NULL) 
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	while(send(sock, buf, len, 0) < 0) {
		if(errno == EINTR) continue;
		CWNetworkRaiseSystemError(CW_ERROR_SENDING);
	}
	
	return CW_TRUE;
}

// receive a datagram on an connected UDP socket (blocking). Unsafe means that we don't use DTLS
CWBool CWNetworkReceiveUnsafeConnected(CWSocket sock, char *buf, int len, int *readBytesPtr) {
	
	if(buf == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	while((*readBytesPtr = recv(sock, buf, len, 0)) < 0) {
		if(errno == EINTR) continue;
		CWNetworkRaiseSystemError(CW_ERROR_RECEIVING);
	}
	
	return CW_TRUE;
}

// receive a datagram on an unconnected UDP socket (blocking). Unsafe means that we don't use DTLS
CWBool CWNetworkReceiveUnsafe(CWSocket sock, char *buf, int len, int flags, CWNetworkLev4Address *addrPtr, int *readBytesPtr) {
	socklen_t addrLen = sizeof(CWNetworkLev4Address);
	
	if(buf == NULL || addrPtr == NULL || readBytesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	while((*readBytesPtr = recvfrom(sock, buf, len, flags, (struct sockaddr*)addrPtr, &addrLen)) < 0) {
		if(errno == EINTR) continue;
		CWNetworkRaiseSystemError(CW_ERROR_RECEIVING);
	}
	
	return CW_TRUE;
}

// init network for client
CWBool CWNetworkInitSocketClient(CWSocket *sockPtr, CWNetworkLev4Address *addrPtr) {
	int yes = 1;
	
	// NULL addrPtr means that we don't want to connect to a specific address
	if(sockPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
#ifdef IPV6
    if(((*sockPtr)=socket((gNetworkPreferredFamily == CW_IPv4) ? AF_INET : AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
#else
	if(((*sockPtr)=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
#endif
		CWNetworkRaiseSystemError(CW_ERROR_CREATING);
	}

	if(addrPtr != NULL) {
		CWUseSockNtop(((struct sockaddr*)addrPtr), CWDebugLog(str););

		if(connect((*sockPtr), ((struct sockaddr*)addrPtr), CWNetworkGetAddressSize(addrPtr)) < 0) {
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
	}

	// allow sending broadcast packets
	setsockopt(*sockPtr, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));
	
	return CW_TRUE;
}

// wrapper for select
CWBool CWNetworkTimedPollRead(CWSocket sock, struct timeval *timeout) {
	int r;
	
	fd_set fset;
	
	if(timeout == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	FD_ZERO(&fset);
	FD_SET(sock, &fset);

	if((r = select(sock+1, &fset, NULL, NULL, timeout)) == 0) {
		CWDebugLog("Select Time Expired");
		return CWErrorRaise(CW_ERROR_TIME_EXPIRED, NULL);
	} else if (r < 0) {
		CWDebugLog("Select Error");
		if(errno == EINTR){
			CWDebugLog("Select Interrupted by signal");
			return CWErrorRaise(CW_ERROR_INTERRUPTED, NULL);
		}
		CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
	}
	
	return CW_TRUE;
}

// given an host int hte form of C string (e.g. "192.168.1.2" or "localhost"), returns the address
CWBool CWNetworkGetAddressForHost(char *host, CWNetworkLev4Address *addrPtr) {
	struct addrinfo hints, *res, *ressave;
	char serviceName[5];
	CWSocket sock;
	
	if(host == NULL || addrPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CW_ZERO_MEMORY(&hints, sizeof(struct addrinfo));
	
#ifdef IPv6
	if(gNetworkPreferredFamily == CW_IPv6) {
		hints.ai_family = AF_INET6;
		hints.ai_flags = AI_V4MAPPED;
	} else {
		hints.ai_family = AF_INET;
	}
#else
	hints.ai_family = AF_INET;
#endif
	hints.ai_socktype = SOCK_DGRAM;
	
	snprintf(serviceName, 5, "%d", CW_CONTROL_PORT); // endianness will be handled by getaddrinfo
	
	if (getaddrinfo(host, serviceName, &hints, &res) !=0 ) {
		return CWErrorRaise(CW_ERROR_GENERAL, "Can't resolve hostname");
	}
	
	ressave = res;
	
	do {
		if((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
			continue; // try next address
		}
		
		break; // success
	} while ( (res = res->ai_next) != NULL);
	
	close(sock);
	
	if(res == NULL) { // error on last iteration
		CWNetworkRaiseSystemError(CW_ERROR_CREATING);
	}
	
	CW_COPY_NET_ADDR_PTR(addrPtr, (res->ai_addr));
	
	freeaddrinfo(ressave);
	
	return CW_TRUE;
}
