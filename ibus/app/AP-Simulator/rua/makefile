ASN_MODULE_SOURCES=	\
	Criticality.c	\
	Presence.c	\
	ProcedureCode.c	\
	PrivateIE-ID.c	\
	ProtocolIE-ID.c	\
	TriggeringMessage.c	\
	ProtocolIE-Container.c	\
	ProtocolIE-Single-Container.c	\
	ProtocolIE-Field.c	\
	ProtocolIE-ContainerList.c	\
	ProtocolExtensionContainer.c	\
	ProtocolExtensionField.c	\
	PrivateIE-Container.c	\
	PrivateIE-Field.c	\
	CN-DomainIndicator.c	\
	CSGMembershipStatus.c	\
	Establishment-Cause.c	\
	Context-ID.c	\
	IntraDomainNasNodeSelector.c	\
	Gsm-map-IDNNS.c	\
	Ansi-41-IDNNS.c	\
	RANAP-Message.c	\
	RoutingParameter.c	\
	Cause.c	\
	CauseRadioNetwork.c	\
	CauseTransport.c	\
	CauseProtocol.c	\
	CauseMisc.c	\
	CriticalityDiagnostics.c	\
	CriticalityDiagnostics-IE-List.c	\
	TypeOfError.c	\
	Connect.c	\
	DirectTransfer.c	\
	Disconnect.c	\
	ConnectionlessTransfer.c	\
	ErrorIndication.c	\
	PrivateMessage.c	\
	RUA-PDU.c	\
	InitiatingMessage.c	\
	SuccessfulOutcome.c	\
	UnsuccessfulOutcome.c

ASN_MODULE_HEADERS=	\
	Criticality.h	\
	Presence.h	\
	ProcedureCode.h	\
	PrivateIE-ID.h	\
	ProtocolIE-ID.h	\
	TriggeringMessage.h	\
	ProtocolIE-Container.h	\
	ProtocolIE-Single-Container.h	\
	ProtocolIE-Field.h	\
	ProtocolIE-ContainerList.h	\
	ProtocolExtensionContainer.h	\
	ProtocolExtensionField.h	\
	PrivateIE-Container.h	\
	PrivateIE-Field.h	\
	CN-DomainIndicator.h	\
	CSGMembershipStatus.h	\
	Establishment-Cause.h	\
	Context-ID.h	\
	IntraDomainNasNodeSelector.h	\
	Gsm-map-IDNNS.h	\
	Ansi-41-IDNNS.h	\
	RANAP-Message.h	\
	RoutingParameter.h	\
	rua-Cause.h	\
	rua-CauseRadioNetwork.h	\
	rua-CauseTransport.h	\
	rua-CauseProtocol.h	\
	rua-CauseMisc.h	\
	CriticalityDiagnostics.h	\
	CriticalityDiagnostics-IE-List.h	\
	TypeOfError.h	\
	Connect.h	\
	DirectTransfer.h	\
	Disconnect.h	\
	ConnectionlessTransfer.h	\
	ErrorIndication.h	\
	PrivateMessage.h	\
	RUA-PDU.h	\
	InitiatingMessage.h	\
	SuccessfulOutcome.h	\
	UnsuccessfulOutcome.h
	
#ASN_CONVERTER_SOURCES = main_rua.c

# This file may be used as an input for make(3)
# Remove the lines below to convert it into a pure .am file
#TARGET = rua
CFLAGS += -I../inc
CFLAGS += -I/opt/mipslibs/usr/include
LDFLAG=-L/opt/mipslibs/usr/lib 
LDFLAG+=-lsctp
LDFLAG+=-L../lib 
LDFLAG+=-lasn1c
CC = mips-linux-gnu-gcc -Werror
OBJS=${ASN_MODULE_SOURCES:.c=.o} 

.SUFFIXES:
.SUFFIXES: .c .o
all: ${OBJS}
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
#	rm -f $(TARGET)
	rm -f $(OBJS)
