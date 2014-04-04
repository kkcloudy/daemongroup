ifndef CC
#CC=gcc
#CC = mips-linux-gnu-gcc
endif
CC = mips-linux-gnu-gcc
CFLAGS = -MMD -O2 -Wall -g -lrt
#LDFLAGS = -L../pub  -lpthread -ldl -D_REENTRANT -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib
# define HOSTAPD_DUMP_STATE to include SIGUSR1 handler for dumping state to
# a file (undefine it, if you want to save in binary size)
#CFLAGS += -DHOSTAPD_DUMP_STATE
#CFLAGS += -D_D_WIRED_
CFLAGS += -D_AC_BAK_UDP_
CFLAGS += -D_DISTRIBUTION_
CFLAGS += -I.
CFLAGS += -I./cipher
CFLAGS += -I./utils
CFLAGS += -I./common
CFLAGS += -I$(ACCAPI_DIR)
CFLAGS += -I./include
CFLAGS += -I./wapi
CFLAGS += -I./wapi/include
CFLAGS += -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
CFLAGS += $(DBUS_INCLUDE)
CFLAGS += -Werror 
# Uncomment following line and set the path to your kernel tree include
# directory if your C library does not include all header files.
# CFLAGS += -DUSE_KERNEL_HEADERS -I/usr/src/linux/include
SUBDIRS=common cipher drivers hlr_auc_gw eapol_supp ASDEAPMethod eap_peer         eap_server l2_packet ASDRadius rsn_supp tls utils
-include asdconfig

ifndef ASD_OS
ifdef ASD_NATIVE_WINDOWS
ASD_OS=win32
else
ASD_OS=unix
endif
endif

ifeq ($(ASD_OS), internal)
CFLAGS += -DOS_NO_C_LIB_DEFINES
endif

ifdef ASD_NATIVE_WINDOWS
CFLAGS += -DASD_NATIVE_WINDOWS
LIBS += -lws2_32
endif

OBJS =	asd.o ASD8021XOp.o ASDEapolSM.o \
	ASD80211Op.o config.o ASD80211AuthOp.o ASDAccounting.o \
	ASDStaInfo.o ASDWPAOp.o ASDCtrlIface.o \
	ASDCallbacks.o ASDPreauth.o ASDPMKCache.o ASDBeaconOp.o \
	ASDHWInfo.o ASDWme.o ASDApList.o reconfig.o \
	ASDMlme.o vlan_init.o ASD80211HOp.o ASDWPAAuthIE.o \
	asd_bak.o ASDDbus_handler.o ASDDbus.o ASDPMKSta.o ASDPreauthBSS.o \
	roaming_sta.o \
	cert_auth.o cert_info.o certupdate.o init.o Key_neg.o \
	none_cert.o pack.o proc.o raw_socket.o wai_sta.o x509_cert.o \
	alg_comm.o ecc_crypt.o debug.o ASDDbusPathInit.o\
	Inter_AC_Roaming.o ASDNetlinkArpOp.o ASDPkcs12.o ASDPkcs12App.o StaFlashDisconnOp.o\
        ASDStaManage.o ASDEAPAuth.o ASDDhcp.o
      


OBJS += ./utils/circle.o
OBJS += ./utils/common.o
OBJS += ./utils/wpa_debug.o
OBJS += ./utils/wpabuf.o
OBJS += ./utils/os_$(ASD_OS).o
OBJS += ./utils/ip_addr.o

OBJS += ./common/wpa_common.o

OBJS += ./ASDRadius/radius.o
OBJS += ./ASDRadius/radius_client.o

OBJS += ./cipher/md5.o
OBJS += ./cipher/rc4.o
OBJS += ./cipher/md4.o
OBJS += ./cipher/sha1.o
OBJS += ./cipher/des.o
OBJS += ./cipher/aes_wrap.o
OBJS += ./cipher/aes.o

HOBJS=./hlr_auc_gw/hlr_auc_gw.o ./utils/common.o ./utils/wpa_debug.o ./utils/os_$(ASD_OS).o ./hlr_auc_gw/milenage.o ./cipher/aes_wrap.o ./cipher/aes.o

CFLAGS += -DASD_CTRL_IFACE -DASD_CTRL_IFACE_UNIX

LDFLAGS = -L./wapi  -lpthread -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib -lpthread
LDFLAGS += -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib -liptc 
LIBS_WAPI += -lwcrypto  -lsms4
#LDFLAGS = -L../pub  -lpthread -ldl -D_REENTRANT -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

ifdef ASD_IAPP
CFLAGS += -DASD_IAPP
OBJS += ASDIappOp.o
endif

ifdef ASD_RSN_PREAUTH
CFLAGS += -DASD_RSN_PREAUTH
ASD_L2_PACKET=y
endif

ifdef ASD_PEERKEY
CFLAGS += -DASD_PEERKEY
OBJS += peerkey.o
endif

ifdef ASD_IEEE80211W
CFLAGS += -DASD_IEEE80211W
NEED_SHA256=y
endif

ifdef ASD_IEEE80211R
CFLAGS += -DASD_IEEE80211R
OBJS += ASDWPAFtOp.o
NEED_SHA256=y
endif

ifdef ASD_IEEE80211N
CFLAGS += -DASD_IEEE80211N
endif


ifdef ASD_CALLBACK_ASD
CFLAGS += -DASD_CALLBACK_ASD
OBJS += ASDCallback_asd.o
endif

ifdef ASD_CALLBACK_WIRED
CFLAGS += -DASD_CALLBACK_WIRED
OBJS += ASDCallback_wired.o
endif


ifdef ASD_L2_PACKET
ifdef ASD_DNET_PCAP
ifdef ASD_L2_FREEBSD
LIBS += -lpcap
OBJS += ./l2_packet/l2_packet_freebsd.o
else
LIBS += -ldnet -lpcap
OBJS += ./l2_packet/l2_packet_pcap.o
endif
else
OBJS += ./l2_packet/l2_packet_linux.o
endif
endif


ifdef ASD_EAP_MD5
CFLAGS += -DEAP_MD5
OBJS += ./ASDEAPMethod/eap_md5.o
CHAP=y
endif

ifdef ASD_EAP_TLS
CFLAGS += -DEAP_TLS
OBJS += ./ASDEAPMethod/eap_tls.o
TLS_FUNCS=y
endif

ifdef ASD_EAP_PEAP
CFLAGS += -DEAP_PEAP
OBJS += ./ASDEAPMethod/eap_peap.o
TLS_FUNCS=y
ASD_EAP_TLV=y
ASD_EAP_MSCHAPV2=y
endif

ifdef ASD_EAP_TTLS
CFLAGS += -DEAP_TTLS
OBJS += ./ASDEAPMethod/eap_ttls.o
TLS_FUNCS=y
CHAP=y
endif

ifdef ASD_EAP_MSCHAPV2
CFLAGS += -DEAP_MSCHAPv2
OBJS += ./ASDEAPMethod/eap_mschapv2.o
MS_FUNCS=y
endif

ifdef ASD_EAP_GTC
CFLAGS += -DEAP_GTC
OBJS += ./ASDEAPMethod/eap_gtc.o
endif

ifdef ASD_EAP_SIM
CFLAGS += -DEAP_SIM
OBJS += ./ASDEAPMethod/eap_sim.o
ASD_EAP_SIM_COMMON=y
endif

ifdef ASD_EAP_AKA
CFLAGS += -DEAP_AKA
OBJS += ./ASDEAPMethod/eap_aka.o
ASD_EAP_SIM_COMMON=y
endif

ifdef ASD_EAP_SIM_COMMON
OBJS += ./ASDEAPMethod/eap_sim_common.o
# Example EAP-SIM/AKA interface for GSM/UMTS authentication. This can be
# replaced with another file implementating the interface specified in
# eap_sim_db.h.
OBJS += ./ASDEAPMethod/eap_sim_db.o
NEED_FIPS186_2_PRF=y
endif

ifdef ASD_EAP_PAX
CFLAGS += -DEAP_PAX
OBJS += ./ASDEAPMethod/eap_pax.o ./ASDEAPMethod/eap_pax_common.o
endif

ifdef ASD_EAP_PSK
CFLAGS += -DEAP_PSK
OBJS += ./ASDEAPMethod/eap_psk.o ./ASDEAPMethod/eap_psk_common.o
endif

ifdef ASD_EAP_SAKE
CFLAGS += -DEAP_SAKE
OBJS += ./ASDEAPMethod/eap_sake.o ./ASDEAPMethod/eap_sake_common.o
endif

ifdef ASD_EAP_GPSK
CFLAGS += -DEAP_GPSK
OBJS += ./ASDEAPMethod/eap_gpsk.o ./ASDEAPMethod/eap_gpsk_common.o
ifdef ASD_EAP_GPSK_SHA256
CFLAGS += -DEAP_GPSK_SHA256
endif
NEED_SHA256=y
endif

ifdef ASD_EAP_VENDOR_TEST
CFLAGS += -DEAP_VENDOR_TEST
OBJS += ./ASDEAPMethod/eap_vendor_test.o
endif

ifdef ASD_EAP_TLV
CFLAGS += -DEAP_TLV
OBJS += ./ASDEAPMethod/eap_tlv.o
endif

ifdef ASD_EAP_FAST
CFLAGS += -DEAP_FAST
OBJS += ./ASDEAPMethod/eap_fast.o
TLS_FUNCS=y
NEED_T_PRF=y
endif

ifdef ASD_EAP_IKEV2
CFLAGS += -DEAP_IKEV2
OBJS += ./ASDEAPMethod/eap_ikev2.o ./ASDEAPMethod/ikev2.o
OBJS += ./ASDEAPMethod/eap_ikev2_common.o ./ASDEAPMethod/ikev2_common.o
NEED_DH_GROUPS=y
endif

# Basic EAP functionality is needed for EAPOL
OBJS += ./ASDEAPMethod/eap.o
OBJS += ./ASDEAPMethod/eap_common.o
OBJS += ./ASDEAPMethod/eap_methods.o
OBJS += ./ASDEAPMethod/eap_identity.o

ifdef ASD_EAP
CFLAGS += -DEAP_SERVER
endif

ifndef ASD_TLS
ASD_TLS=openssl
endif

ifeq ($(ASD_TLS), internal)
ifndef ASD_CRYPTO
ASD_CRYPTO=internal
endif
endif
ifeq ($(ASD_CRYPTO), libtomcrypt)
CFLAGS += -DASD_INTERNAL_X509
endif
ifeq ($(ASD_CRYPTO), internal)
CFLAGS += -DASD_INTERNAL_X509
endif


ifdef TLS_FUNCS
# Shared TLS functions (needed for EAP_TLS, EAP_PEAP, and EAP_TTLS)
CFLAGS += -DEAP_TLS_FUNCS
OBJS += ./ASDEAPMethod/eap_tls_common.o
ifeq ($(ASD_TLS), openssl)
OBJS += ./cipher/tls_openssl.o
LIBS += -lwssl -lwcrypto -lz
LIBS_p += -lwcrypto
LIBS_h += -lwcrypto
endif
ifeq ($(ASD_TLS), gnutls)
OBJS += ./cipher/tls_gnutls.o
LIBS += -lgnutls -lgcrypt -lgpg-error
LIBS_p += -lgcrypt
LIBS_h += -lgcrypt
endif
ifdef ASD_GNUTLS_EXTRA
CFLAGS += -DASD_GNUTLS_EXTRA
LIBS += -lgnutls-extra
endif
ifeq ($(ASD_TLS), internal)
OBJS += ./cipher/tls_internal.o
OBJS += ./tls/tlsv1_common.o ./tls/tlsv1_record.o
OBJS += ./tls/tlsv1_cred.o ./tls/tlsv1_server.o
OBJS += ./tls/tlsv1_server_write.o ./tls/tlsv1_server_read.o
OBJS += ./tls/asn1.o ./tls/x509v3.o
OBJS_p += ./tls/asn1.o
OBJS_p += ./cipher/rc4.o ./cipher/aes_wrap.o ./cipher/aes.o
NEED_BASE64=y
CFLAGS += -DASD_TLS_INTERNAL
CFLAGS += -DASD_TLS_INTERNAL_SERVER
ifeq ($(ASD_CRYPTO), internal)
ifdef ASD_INTERNAL_LIBTOMMATH
CFLAGS += -DASD_INTERNAL_LIBTOMMATH
else
LIBS += -ltommath
LIBS_p += -ltommath
endif
endif
ifeq ($(ASD_CRYPTO), libtomcrypt)
LIBS += -ltomcrypt -ltfm
LIBS_p += -ltomcrypt -ltfm
endif
endif
NEED_CRYPTO=y
else
OBJS += ./cipher/tls_none.o
endif

ifdef ASD_PKCS12
CFLAGS += -DPKCS12_FUNCS
endif

ifdef MS_FUNCS
OBJS += ./cipher/ms_funcs.o
NEED_CRYPTO=y
endif

ifdef CHAP
OBJS += ./ASDEAPMethod/chap.o
endif

ifdef NEED_CRYPTO
ifndef TLS_FUNCS
ifeq ($(ASD_TLS), openssl)
LIBS += -lwcrypto
LIBS_p += -lwcrypto
LIBS_h += -lwcrypto
endif
ifeq ($(ASD_TLS), gnutls)
LIBS += -lgcrypt
LIBS_p += -lgcrypt
LIBS_h += -lgcrypt
endif
ifeq ($(ASD_TLS), internal)
ifeq ($(ASD_CRYPTO), libtomcrypt)
LIBS += -ltomcrypt -ltfm
LIBS_p += -ltomcrypt -ltfm
endif
endif
endif
ifeq ($(ASD_TLS), openssl)
OBJS += ./cipher/crypto_openssl.o
OBJS_p += ./cipher/crypto_openssl.o
HOBJS += ./cipher/crypto_openssl.o
ASD_INTERNAL_SHA256=y
endif
ifeq ($(ASD_TLS), gnutls)
OBJS += ./cipher/crypto_gnutls.o
OBJS_p += ./cipher/crypto_gnutls.o
HOBJS += ./cipher/crypto_gnutls.o
ASD_INTERNAL_SHA256=y
endif
ifeq ($(ASD_TLS), internal)
ifeq ($(ASD_CRYPTO), libtomcrypt)
OBJS += ./cipher/crypto_libtomcrypt.o
OBJS_p += ./cipher/crypto_libtomcrypt.o
ASD_INTERNAL_SHA256=y
endif
ifeq ($(ASD_CRYPTO), internal)
OBJS += ./cipher/crypto_internal.o ./tls/rsa.o ./tls/bignum.o
OBJS_p += ./cipher/crypto_internal.o ./tls/rsa.o ./tls/bignum.o
CFLAGS += -DASD_CRYPTO_INTERNAL
ASD_INTERNAL_AES=y
ASD_INTERNAL_DES=y
ASD_INTERNAL_SHA1=y
ASD_INTERNAL_MD4=y
ASD_INTERNAL_MD5=y
ASD_INTERNAL_SHA256=y
endif
endif
else
ASD_INTERNAL_AES=y
ASD_INTERNAL_SHA1=y
ASD_INTERNAL_MD5=y
ASD_INTERNAL_SHA256=y
endif

ifdef ASD_INTERNAL_AES
CFLAGS += -DINTERNAL_AES
endif
ifdef ASD_INTERNAL_SHA1
CFLAGS += -DINTERNAL_SHA1
endif
ifdef ASD_INTERNAL_SHA256
CFLAGS += -DINTERNAL_SHA256
endif
ifdef ASD_INTERNAL_MD5
CFLAGS += -DINTERNAL_MD5
endif
ifdef ASD_INTERNAL_MD4
CFLAGS += -DINTERNAL_MD4
endif
ifdef ASD_INTERNAL_DES
CFLAGS += -DINTERNAL_DES
endif

ifdef NEED_SHA256
OBJS += ./cipher/sha256.o
endif

ifdef NEED_DH_GROUPS
OBJS += ./cipher/dh_groups.o
endif

ifndef NEED_FIPS186_2_PRF
CFLAGS += -DASD_NO_FIPS186_2_PRF
endif

ifndef NEED_T_PRF
CFLAGS += -DASD_NO_T_PRF
endif

ifdef ASD_RADIUS_SERVER
CFLAGS += -DRADIUS_SERVER
OBJS += ./ASDRadius/radius_server.o
endif

#ifdef ASD_IPV6
CFLAGS += -DASD_IPV6
#endif

ifdef ASD_FULL_DYNAMIC_VLAN
CFLAGS += -DASD_FULL_DYNAMIC_VLAN
endif

ifdef NEED_BASE64
OBJS += ./utils/base64.o
endif

ALL=asd 

all: verify_config $(ALL)

verify_config:
	@if [ ! -r asdconfig ]; then \
		echo 'Building asd requires a configuration file'; \
		echo '(asdconfig). See README for more instructions. You can'; \
		echo 'run "cp defconfig asdconfig" to create an example'; \
		echo 'configuration.'; \
		exit 1; \
	fi

install: all
	for i in $(ALL); do cp $$i /usr/local/bin/$$i; done

asd: $(OBJS)
	[ -f wapi/libwssl.so ] || ln -s libwssl.so.0.9.8 wapi/libwssl.so
	[ -f wapi/libwcrypto.so ] || ln -s libwcrypto.so.0.9.8 wapi/libwcrypto.so
	$(CC) -g -Wall -I /opt/mipslibs/usr/include -c rdc_handle.c -o rdc_handle.o
#	$(CC) -g -Wall -I /opt/mipslibs/usr/include -c asd_iptables.c -o asd_iptables.o
	$(CC) rdc_handle.o  -o asd $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS_WAPI) $(LIBS) $(DBUS_LIBS)




NOBJS = nt_password_hash.o ./cipher/ms_funcs.o ./cipher/sha1.o ./cipher/rc4.o ./cipher/md5.o
NOBJS += ./cipher/crypto_openssl.o ./utils/os_$(ASD_OS).o
ifdef TLS_FUNCS
LIBS_n += -lwcrypto
endif

nt_password_hash: $(NOBJS)
	$(CC) -o nt_password_hash $(NOBJS) $(LIBS_n)

hlr_auc_gw: $(HOBJS)
	$(CC) -o hlr_auc_gw $(HOBJS) $(LIBS_h)

clean:
#	$(MAKE) -C . clean
	rm -f core *~ *.o asd 
	rm -f *.d
	for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) -C $$d clean; done
	rm -f *~

%.eps: %.fig
	fig2dev -L eps $*.fig $*.eps

%.png: %.fig
	fig2dev -L png -m 3 $*.fig | pngtopnm | pnmscale 0.4 | pnmtopng \
		> $*.png



TEST_SRC_MILENAGE = ./hlr_auc_gw/milenage.c ./cipher/aes_wrap.c ./cipher/aes.c ./utils/common.c ./utils/wpa_debug.o ./utils/os_$(ASD_OS).c
test-milenage: $(TEST_SRC_MILENAGE)
	$(CC) -o test-milenage -Wall -Werror $(TEST_SRC_MILENAGE) \
		-DTEST_MAIN_MILENAGE -I. -DINTERNAL_AES \
		-I./cipher -I./utils
	./test-milenage
	rm test-milenage



-include $(OBJS:%.o=%.d)
