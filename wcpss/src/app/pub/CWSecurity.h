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
* CWSecurity.h
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


#ifndef __CAPWAP_CWSecurity_HEADER__
#define __CAPWAP_CWSecurity_HEADER__
#ifndef CW_NO_DTLS
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

typedef SSL_CTX *CWSecurityContext;
typedef SSL *CWSecuritySession;

#define		CWSecuritySetPeerForSession(session, addrPtr)	BIO_ctrl((session)->rbio, BIO_CTRL_DGRAM_SET_PEER, 1, (addrPtr))

CWBool CWSecurityInitLib(void);
CWBool CWSecurityInitSessionClient(CWSocket sock, CWNetworkLev4Address *addrPtr, CWSafeList packetReceiveList, CWSecurityContext ctx, CWSecuritySession *sessionPtr, int *PMTUPtr);
CWBool CWSecuritySend(CWSecuritySession session, const char *buf, int len);
CWBool CWSecurityReceive(CWSecuritySession session, char *buf, int len, int *readBytesPtr);

CWBool CWSecurityInitContext(CWSecurityContext *ctxPtr, const char *caList, const char *keyfile, const char *passw, CWBool isClient, int (*hackPtr)(void *)) ;
void CWSecurityDestroyContext(CWSecurityContext ctx);
void CWSecurityDestroySession(CWSecuritySession s);

BIO* BIO_new_memory(CWSocket sock, CWNetworkLev4Address* pSendAddress, CWSafeList* pRecvAddress);
#endif
#endif
