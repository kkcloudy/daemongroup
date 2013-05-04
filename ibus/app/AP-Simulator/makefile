MAKE=make
HNBAP_DIR="hnbap"
RUA_DIR="rua"

HNBAP_OBJ=\
./hnbap/Access-stratum-release-indicator.o \
./hnbap/AltitudeAndDirection.o \
./hnbap/BackoffTimer.o \
./hnbap/CGI.o \
./hnbap/CI.o \
./hnbap/CSG-ID.o \
./hnbap/CSG-Indicator.o \
./hnbap/CSGMembershipUpdate.o \
./hnbap/CauseRadioNetwork.o \
./hnbap/CellIdentity.o \
./hnbap/CriticalityDiagnostics-IE-List.o \
./hnbap/CriticalityDiagnostics.o \
./hnbap/ESN.o \
./hnbap/ErrorIndication.o \
./hnbap/GeographicalCoordinates.o \
./hnbap/GeographicalLocation.o \
./hnbap/HNB-Cell-Access-Mode.o \
./hnbap/HNB-Identity-Info.o \
./hnbap/HNB-Identity.o \
./hnbap/HNB-Location-Information.o \
./hnbap/HNBAP-PDU.o \
./hnbap/HNBDe-Register.o \
./hnbap/HNBRegisterAccept.o \
./hnbap/HNBRegisterReject.o \
./hnbap/HNBRegisterRequest.o \
./hnbap/IMEI.o \
./hnbap/IMSI.o \
./hnbap/IMSIDS41.o \
./hnbap/IMSIESN.o \
./hnbap/IP-Address.o \
./hnbap/Ipv4Address.o \
./hnbap/Ipv6Address.o \
./hnbap/LAC.o \
./hnbap/LAI.o \
./hnbap/MacroCellID.o \
./hnbap/MacroCoverageInformation.o \
./hnbap/MuxPortNumber.o \
./hnbap/PLMNidentity.o \
./hnbap/PTMSI.o \
./hnbap/PTMSIRAI.o \
./hnbap/PrivateIE-Container.o \
./hnbap/PrivateMessage.o \
./hnbap/ProtocolExtensionContainer.o \
./hnbap/ProtocolIE-Container.o \
./hnbap/RAC.o \
./hnbap/RAI.o \
./hnbap/RNC-ID.o \
./hnbap/Registration-Cause.o \
./hnbap/SAC.o \
./hnbap/TMSIDS41.o \
./hnbap/TMSILAI.o \
./hnbap/UE-Capabilities.o \
./hnbap/UE-Identity.o \
./hnbap/UEDe-Register.o \
./hnbap/UERegisterAccept.o \
./hnbap/UERegisterReject.o \
./hnbap/UERegisterRequest.o \
./hnbap/UTRANCellID.o \


RUA_OBJ=\
./rua/Ansi-41-IDNNS.o \
./rua/CN-DomainIndicator.o \
./rua/CSGMembershipStatus.o \
./rua/Cause.o \
./rua/CauseMisc.o \
./rua/CauseProtocol.o \
./rua/CauseRadioNetwork.o \
./rua/CauseTransport.o \
./rua/Connect.o \
./rua/ConnectionlessTransfer.o \
./rua/Context-ID.o \
./rua/Criticality.o \
./rua/CriticalityDiagnostics-IE-List.o \
./rua/CriticalityDiagnostics.o \
./rua/DirectTransfer.o \
./rua/Disconnect.o \
./rua/ErrorIndication.o \
./rua/Establishment-Cause.o \
./rua/Gsm-map-IDNNS.o \
./rua/InitiatingMessage.o \
./rua/IntraDomainNasNodeSelector.o \
./rua/Presence.o \
./rua/PrivateIE-Container.o \
./rua/PrivateIE-Field.o \
./rua/PrivateIE-ID.o \
./rua/PrivateMessage.o \
./rua/ProcedureCode.o \
./rua/ProtocolExtensionContainer.o \
./rua/ProtocolExtensionField.o \
./rua/ProtocolIE-Container.o \
./rua/ProtocolIE-ContainerList.o \
./rua/ProtocolIE-Field.o \
./rua/ProtocolIE-ID.o \
./rua/ProtocolIE-Single-Container.o \
./rua/RANAP-Message.o \
./rua/RUA-PDU.o \
./rua/RoutingParameter.o \
./rua/SuccessfulOutcome.o \
./rua/TriggeringMessage.o \
./rua/TypeOfError.o \
./rua/UnsuccessfulOutcome.o


ASN_CONVERTER_SOURCES=test.c

TARGET = test
CFLAGS += -I./inc
CFLAGS += -I/opt/mipslibs/usr/include
CFLAGS += -I./hnbap
CFLAGS += -I./rua

LDFLAG=-L/opt/mipslibs/usr/lib 
LDFLAG+=-lsctp
LDFLAG+=-L./lib 
LDFLAG+=-lasn1c
CC = mips-linux-gnu-gcc -Werror
OBJS=${ASN_CONVERTER_SOURCES:.c=.o}

all: $(TARGET)

$(TARGET): ${OBJS} HNBAP RUA
	$(CC) $(CFLAGS) $(LDFLAG) -o $(TARGET) ${OBJS} ${HNBAP_OBJ} ${RUA_OBJ}
HNBAP:
	${MAKE} -C ${HNBAP_DIR}
RUA:
	${MAKE} -C ${RUA_DIR}
  
.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(LDFLAG) -o $@ -c $<

clean: cleanhnbap cleanrua
	rm -f $(TARGET)
	rm -f $(TARGET).o
	
cleanhnbap:
	${MAKE} -C ${HNBAP_DIR} clean
  
cleanrua:
	${MAKE} -C ${RUA_DIR} clean
