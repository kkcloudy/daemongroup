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
* ACProtocol_user.c
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

 
#include "CWAC.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

__inline__ CWBool CWACSupportIPv6() {
	return (gNetworkPreferredFamily == CW_IPv6);
}

__inline__ char * CWACGetName() {
	return gACName;
}

__inline__ int CWACGetStations() {
	return gActiveStations;
}

__inline__ int CWACGetLimit() {
	return gLimit;
}

__inline__ int CWACGetActiveWTPs() {
	int tmp;
	if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) return 0;
		tmp = gActiveWTPs;
	CWThreadMutexUnlock(&gActiveWTPsMutex);
	
	return tmp;
}

__inline__ int CWACGetMaxWTPs() {
	return WTP_NUM;
}

__inline__ int CWACGetSecurity() {
	return gACDescriptorSecurity;
}

__inline__ int CWACGetRMACField() {
	return gRMACField;
}

__inline__ int CWACGetWirelessField() {
	return gWirelessField;
}

__inline__ int CWACGetDTLSPolicy() {
	return gDTLSPolicy;
}

__inline__ int CWACGetHWVersion() {
	return gACHWVersion;
}

__inline__ int CWACGetSWVersion() {
	return gACSWVersion;
}

__inline__ int CWACGetInterfacesCount() {
	return gInterfacesCount;
}
__inline__ int CWACGetInterfacesIpv4Count() {
	return gInterfacesCountIpv4;
}
__inline__ int CWACGetInterfacesIpv6Count() {
	return gInterfacesCountIpv6;
}


__inline__ int CWACGetInterfaceIPv4AddressAtIndex(int i) {
	struct sockaddr_in *addrPtr;
	
	if(gNetworkPreferredFamily == CW_IPv4) {
		addrPtr = (struct sockaddr_in *) &(gInterfaces[i].addr);
	} else {
		addrPtr = (struct sockaddr_in *) &(gInterfaces[i].addrIPv4);
	}
	
	return ntohl(addrPtr->sin_addr.s_addr);
}

__inline__ char *CWACGetInterfaceIPv6AddressAtIndex(int i) {
	struct sockaddr_in6 *addrPtr;
	
	addrPtr = (struct sockaddr_in6 *) &(gInterfaces[i].addr);
	
	return (char*) addrPtr->sin6_addr.s6_addr;
}

__inline__ int CWACGetInterfaceWTPCountAtIndex(int i) {
	return gInterfaces[i].WTPCount;
}

CWBool CWACGetVendorInfos(CWACVendorInfos *valPtr) {
	if(valPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	valPtr->vendorInfosCount = 2; 
	CW_CREATE_ARRAY_ERR((valPtr->vendorInfos), valPtr->vendorInfosCount, CWACVendorInfoValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	// my vendor identifier (IANA assigned "SMI Network Management Private Enterprise Code")
	(valPtr->vendorInfos)[0].vendorIdentifier = 65432;
	(valPtr->vendorInfos)[0].type = CW_AC_HARDWARE_VERSION;
	(valPtr->vendorInfos)[0].length = 4; // just one int
	CW_CREATE_OBJECT_SIZE_ERR((((valPtr->vendorInfos)[0]).valuePtr), 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	*(((valPtr->vendorInfos)[0]).valuePtr) = CWACGetHWVersion(); // HW version
	
	// my vendor identifier (IANA assigned "SMI Network Management Private Enterprise Code")
	((valPtr->vendorInfos)[1]).vendorIdentifier = 65432;
	((valPtr->vendorInfos)[1]).type = CW_AC_SOFTWARE_VERSION;
	((valPtr->vendorInfos)[1]).length = 4; // just one int
	CW_CREATE_OBJECT_SIZE_ERR(( ( (valPtr->vendorInfos)[1] ).valuePtr), 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	*(((valPtr->vendorInfos)[1] ).valuePtr) = CWACGetSWVersion(); // SW version
	
	return CW_TRUE;
}

__inline__ void CWACDestroyVendorInfos(CWACVendorInfos *valPtr) {
	int i;
	
	if(valPtr == NULL) return;
	
	for(i = 0; i < valPtr->vendorInfosCount; i++) {
		CW_FREE_OBJECT_WID((valPtr->vendorInfos)[i].valuePtr);
	}
	
	CW_FREE_OBJECT_WID(valPtr->vendorInfos);
}

CWBool CWACGetACIPv4List(int **listPtr, int *countPtr) 
{
	struct in_addr addr;
	
	// TO-DO this func should return the addresses of eventual other ACs in a cluster. Hey, what? What is the WTP
	// supposed to do with that?
	if(listPtr == NULL || countPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	*countPtr = 2;
	
	CW_CREATE_ARRAY_ERR((*listPtr), (*countPtr), int, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	inet_pton(AF_INET, "192.168.1.2", &addr); // TO-DO take the addresses from config file?
	(*listPtr)[0] = addr.s_addr;
	inet_pton(AF_INET, "192.168.1.66", &addr);
	(*listPtr)[1] = addr.s_addr;
	
	return CW_TRUE;
}

CWBool CWACGetACIPv6List(struct in6_addr **listPtr, int *countPtr) 
{
	// TO-DO this func should return the addresses of eventual other ACs in a cluster. Hey, what? What is the WTP
	// supposed to do with that?
	if(listPtr == NULL || countPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	*countPtr = 2;
	
	CW_CREATE_ARRAY_ERR(*listPtr, (*countPtr), struct in6_addr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	inet_pton(AF_INET6, "5f1b:df00:ce3e:e200:0020:0800:2078:e3e3", &((*listPtr)[0])); // TO-DO take the addresses from config file?
	inet_pton(AF_INET6, "5f1b:df00:ce3e:e200:0020:0800:2078:e3e4", &((*listPtr)[1]));
	
	return CW_TRUE;
}

CWBool CWACGetDiscoveryTimer (int *timer)
{
	*timer=gDiscoveryTimer;
	return CW_TRUE;
}

CWBool CWACGetEchoRequestTimer (int *timer)
{
	*timer=gEchoRequestTimer;
	return CW_TRUE;
}

CWBool CWACGetIdleTimeout (int *timer)
{
	*timer=gIdleTimeout;
	return CW_TRUE;
}

/* Il WTP ha la funzione ridefinita */
CWBool CWGetWTPRadiosAdminState(CWRadiosAdminInfo *valPtr) 
{
	int *WTPIndexPtr;	

	if(valPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if((WTPIndexPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		return CW_FALSE;
	}

	valPtr->radiosCount = gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radioCount;
	
	CW_CREATE_ARRAY_ERR(valPtr->radios, valPtr->radiosCount, CWRadioAdminInfoValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	int i;
	for(i=0; i<valPtr->radiosCount; i++)
	{
		(valPtr->radios)[i].ID = gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].radioID;
		(valPtr->radios)[i].state = gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].adminState;
		(valPtr->radios)[i].cause = gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].adminCause;
	}
	
	return CW_TRUE;
}

CWBool CWGetWTPRadiosOperationalState(int radioID, CWRadiosOperationalInfo *valPtr)
{
	int i;
	CWBool found = CW_FALSE;
	int *WTPIndexPtr;

	if(valPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if((WTPIndexPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		return CW_FALSE;
	}

	if(radioID<0) {
		valPtr->radiosCount = gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radioCount;
	
		CW_CREATE_ARRAY_ERR(valPtr->radios, valPtr->radiosCount, CWRadioOperationalInfoValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
		for (i=0; i<valPtr->radiosCount; i++)
		{
			(valPtr->radios)[i].ID =  gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].radioID;
			(valPtr->radios)[i].state =  gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].operationalState;
			(valPtr->radios)[i].cause =  gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].operationalCause;
		}
		return CW_TRUE;	
	}	
	else {
		for (i=0; i<valPtr->radiosCount; i++)
		{
			if(gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].radioID == radioID)
			{
				found = CW_TRUE;
				valPtr->radiosCount = 1;
				CW_CREATE_ARRAY_ERR(valPtr->radios, valPtr->radiosCount, CWRadioOperationalInfoValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				(valPtr->radios)[i].ID =  gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].radioID;
				(valPtr->radios)[i].state =  gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].operationalState;
				(valPtr->radios)[i].cause =  gWTPs[*WTPIndexPtr].WTPProtocolManager.radiosInfo.radiosInfo[i].operationalCause;
				break;
			}
		}
		return found;
	}
}


