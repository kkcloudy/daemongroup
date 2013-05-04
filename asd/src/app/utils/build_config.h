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
* Build_config.h
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

#ifndef BUILD_ASD_H
#define BUILD_ASD_H

/* Insert configuration defines, e.g., #define EAP_MD5, here, if needed. */

#ifdef __SYMBIAN32__
#define OS_NO_C_LIB_DEFINES
#define ASD_ANSI_C_EXTRA
#define ASD_NO_WPA_MSG
#define ASD_NO_asd_LOGGER
#define ASD_NO_STDOUT_DEBUG
#define ASD_BACKEND_FILE
#define INTERNAL_AES
#define INTERNAL_SHA1
#define INTERNAL_MD5
#define INTERNAL_MD4
#define INTERNAL_DES
#define ASD_INTERNAL_LIBTOMMATH
#define ASD_INTERNAL_X509
#define EAP_TLS_FUNCS
#define ASD_TLS_INTERNAL
#define ASD_CRYPTO_INTERNAL
#define IEEE8021X_EAPOL
#define PKCS12_FUNCS
#define EAP_MD5
#define EAP_TLS
#define EAP_MSCHAPv2
#define EAP_PEAP
#define EAP_TTLS
#define EAP_GTC
#define EAP_OTP
#define EAP_LEAP
#define EAP_FAST
#endif /* __SYMBIAN32__ */

#endif /* BUILD_ASD_H */
