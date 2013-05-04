#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include <sys/uio.h>
#include <netinet/sctp.h>  
#include "iuh/Iuh.h"
#include "Iuh_Thread.h"
#include "Iuh_SockOP.h"
#include "Iuh_log.h"
#include "Iuh_DBus.h"
#include "Iuh_IuRecv.h"
#include "Iuh_ManageHNB.h"
#include "Iuh_Msgq.h"
#include "Iuh_DBus_handler.h"
#include "Iuh_Decode.h"
#include "Iuh_Encode.h"


/*****************************************************
** DISCRIPTION:
**          Accept sctp connection
** INPUT:
**          IuhMultiHomedInterface struct
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void IuhSctpAccept(struct IuhMultiHomedInterface *inf){
 
	struct IuhMultiHnbSocket *subinf;	
	int addr_len;
	struct sctp_event_subscribe events;
	subinf = (struct IuhMultiHnbSocket *)malloc(sizeof(struct IuhMultiHnbSocket));
	memset(subinf, 0, sizeof(struct IuhMultiHnbSocket));
	subinf->sock = accept(inf->sock, (struct sockaddr *)&(subinf->addr), &addr_len);
	if(subinf->sock == -1){
		free(subinf);
		subinf = NULL;
		return;
	}
	
	bzero(&events, sizeof (events));
	events.sctp_data_io_event = 1;
	events.sctp_association_event = 1;
	events.sctp_address_event = 1;
	events.sctp_send_failure_event = 1;
	events.sctp_peer_error_event = 1;
	events.sctp_shutdown_event = 1;

	if (setsockopt(subinf->sock, IPPROTO_SCTP, SCTP_EVENTS, &events,sizeof (events)) < 0) {
		perror("setsockopt SCTP_EVENTS");
	}
	
    subinf->if_belong = inf;
	subinf->sock_next = inf->sub_sock;
	inf->sub_sock = subinf;
	inf->sub_sock_count++;
	return;
}


/*****************************************************
** DISCRIPTION:
**          Receive sctp message
** INPUT:
**          socket
** OUTPUT:
**          message buf
**          message length
**          sctp_sndrcvinfo
**          address info
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhSCTPReceive(int sock, char *buf, int len, struct sctp_sndrcvinfo *sinfo, int *readBytesPtr, struct sockaddr* addrPtr) {

  
	socklen_t addrLen = sizeof(struct sockaddr);
	int msg_flags;
	if(buf == NULL || sinfo == NULL || readBytesPtr == NULL) return Iuh_FALSE;
	*readBytesPtr = sctp_recvmsg(sock, (void *)buf, len, (struct sockaddr*)addrPtr, &addrLen, sinfo, &msg_flags);

	iuh_syslog_debug_debug(IUH_DEFAULT, "\n### readBytesPtr = %d ###\n\n",*readBytesPtr);
	if(*readBytesPtr <= 0){
		if(errno == EINTR) return Iuh_FALSE;
		perror("sctp_recvmsg");
		return Iuh_FALSE;
	}
	if(msg_flags  &  MSG_NOTIFICATION){  
		printf(	"msg_flags	(%d),MSG_NOTIFICATION(%d)\n  ",msg_flags,MSG_NOTIFICATION);
		return -1;
	}

	iuh_syslog_info(" ");
	iuh_syslog_info(" ");
	iuh_syslog_info(" ");
	iuh_syslog_info("#################################");
	iuh_syslog_info("###  Receive Message From HNB\n");
	
	return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Send sctp message
** INPUT:
**          socket info
**          message struct
** OUTPUT:
**          null
** RETURN:
**          send message length
*****************************************************/

int IuhSCTPSend(struct IuhMultiHnbSocket * sock_info, IuhProcotolMsg *msgPtr) {
    int ret;
    if((sock_info == NULL) || (msgPtr == NULL)){
        iuh_syslog_err("Sending message is invalid\n");
        return 0;
    }
    socklen_t addrLen = sizeof(struct sockaddr);
    
    iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sock = %d\n",sock_info->sock);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sinfo.sinfo_ppid = %d\n",sock_info->sinfo.sinfo_ppid);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sinfo.sinfo_flags = %d\n",sock_info->sinfo.sinfo_flags);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sinfo.sinfo_stream = %d\n",sock_info->sinfo.sinfo_stream);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sinfo.sinfo_timetolive = %d\n",sock_info->sinfo.sinfo_timetolive);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sinfo.sinfo_context = %d\n",sock_info->sinfo.sinfo_context);
    
    ret = sctp_sendmsg(sock_info->sock, (void *)msgPtr->buf, msgPtr->size, (struct sockaddr *)&sock_info->addr, addrLen, sock_info->sinfo.sinfo_ppid, sock_info->sinfo.sinfo_flags, sock_info->sinfo.sinfo_stream, sock_info->sinfo.sinfo_timetolive, sock_info->sinfo.sinfo_context);
    if(ret == -1){
        perror("server send:");
        iuh_syslog_err("send SCTP message error\n");
        return 0;
    }

	iuh_syslog_info("###  Send Message To HNB \n");
	iuh_syslog_info("#################################\n\n\n");
	
    return 1;
}



/*****************************************************
** DISCRIPTION:
**          Manage incoming packet from hnb, if is unknown hnb, 
**      create new hnb information struct.
** INPUT:
**          socket info
**          buf
**          buf length
**          message type
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void IuhManageIncomingPacket(struct IuhMultiHnbSocket * sock_info, char *buf, int readBytes, int type) 
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"IuhManageIncomingPacket\n");
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->HNBID = %d\n",sock_info->HNBID);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sock = %d\n",sock_info->sock);
    
	msgq qData;	
	if(sock_info->HNBID != 0){ //known hnb
		if((sock_info->HNBID > HNB_DEFAULT_NUM_AUTELAN)||(IUH_HNB[sock_info->HNBID] == NULL)){
		    iuh_syslog_err("Invalid HNB ID\n");
			return;
		}
		
		qData.mqid = (sock_info->HNBID)%THREAD_NUM+1;
		qData.mqinfo.HNBID = sock_info->HNBID;
		qData.mqinfo.type = type;
		qData.mqinfo.u.DataInfo.len = readBytes;
		memset(qData.mqinfo.u.DataInfo.Data, 0, 2048);
		memcpy(qData.mqinfo.u.DataInfo.Data, buf, readBytes);
		iuh_syslog_debug_debug(IUH_DEFAULT,"send msg IuhMsgQid = %d\n",IuhMsgQid);
		if (msgsnd(IuhMsgQid, (msgq *)&qData, sizeof(qData.mqinfo), 0) == -1)
			perror("msgsnd");
	}
	else{ /* unknown  hnb, only has hnb-register request message */
		if(type != HNBAP_DATA){
		    perror("error message type");
		    return;
		}
		else{
		    HNBAP_PDU_t hnbap_pdu;
		    memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
		    iuh_syslog_debug_debug(IUH_DEFAULT,"parse hnbap pdu\n");
		    /* parse hnbap pdu msg or decode hnbap pud msg error */
		    //printf("aaaaaaaaaaaaaa\n");
		    //printf("readbytes = %d\n",readBytes);
		    if(!IuhParseHnbapMessage(buf, readBytes, &hnbap_pdu)){
		        perror("parse HNBAP-PDU error");
		        return;
		    }
		    else{
		        //printf("bbbbbbbbbbbbbbbb\n");
		        InitiatingMessage_t	 *initiatingMessage = &(hnbap_pdu.choice.initiatingMessage);
		        //printf("11\n");
                if((hnbap_pdu.present != HNBAP_PDU_PR_initiatingMessage) ||
                    (initiatingMessage->procedureCode != HNBRegister)){
                    perror("error message type");
                    //printf("hnbap_pdu.present = %d, initiatingMessage->procedureCode = %d\n",hnbap_pdu.present,initiatingMessage->procedureCode);
                    //printf("22\n");
		            return;
                }
		        else{
		            //printf("33\n");
		            int ret;
		            int i;
		            int HnbIndex = 0;
		            HNBAPCause cause;
		            IuhProcotolMsg msgPtr;
		            HnbRegisterRequestValue requestValue;
		            memset(&requestValue, 0, sizeof(HnbRegisterRequestValue));
		            ANY_t *value = &(hnbap_pdu.choice.initiatingMessage.value);
		            
		            if(!IuhDecodeHnbRegisterRequest((char*)value->buf, value->size, &requestValue, &cause)){
		                iuh_syslog_err("decode HNB Registe Request error\n");
		                if(!IuhAssembleRegisterReject(&cause, &msgPtr)){
		                    iuh_syslog_err("assemble HNB Register Reject error\n");
		                    IUH_FREE_HnbRegisterRequestValue(&requestValue);
		                    return;
		                }
		            }
		            else{
		                iuh_syslog_debug_debug(IUH_DEFAULT,"decode ok\n");
		                for(i = 1; i <= HNB_DEFAULT_NUM_AUTELAN; i++) {	
							if(IUH_HNB[i] == NULL){
								HnbIndex = i;
								break;
							}
						}
						
		                if(HnbIndex == 0){
		                    /* there is no free hnb to use */
		                    cause.present = Cause_misc;
		                    cause.choice.misc = Misc_processing_overload;
		                    if(!IuhAssembleRegisterReject(&cause, &msgPtr)){
    		                    iuh_syslog_err("assemble HNB Register Reject error\n");
    		                    IUH_FREE_HnbRegisterRequestValue(&requestValue);
    		                    return;
    		                }
		                }
		                else{
		                    /* create hnb */
		                    ret = IUH_CREATE_NEW_HNB("autohnb", HnbIndex, &requestValue, sock_info->sock);
		                    iuh_syslog_debug_debug(IUH_DEFAULT,"create OK\n");
		                    IUH_FREE_HnbRegisterRequestValue(&requestValue);
		                    iuh_syslog_debug_debug(IUH_DEFAULT,"free ok\n");
		                    if(ret == 0){
		                        if(!IuhAssembleRegisterAccept(HnbIndex, &msgPtr)){
        		                    iuh_syslog_err("assemble HNB Register Accept error\n");
        		                    return;
        		                }
		                    }
		                    else{
		                        cause.present = Cause_misc;
		                        cause.choice.misc = Misc_processing_overload;
		                        if(!IuhAssembleRegisterReject(&cause, &msgPtr)){
        		                    iuh_syslog_err("assemble HNB Register Reject error\n");
        		                    return;
        		                }
		                    }
		                }
		            }
					
		            /* send message */
		            if(!IuhSCTPSend(sock_info, &msgPtr)) {
		                iuh_syslog_err("send message to HNB error\n");
		                return;
		            }
		        }
		    }
		}
	}
    return;
}


/*****************************************************
** DISCRIPTION:
**          Check ppid
** INPUT:
**          ppid
** OUTPUT:
**          null
** RETURN:
**          ppid type
*****************************************************/

int IuhDataTypeCheck(int ppid){
	if(ppid == 20){
		return HNBAP_DATA;
	}else if(ppid == 19){
		return RUA_DATA;
	}else{
		return 0;
	}
}


/*****************************************************
** DISCRIPTION:
**          Check exist sockets, listen these sockets and receive messages
** INPUT:
**          socket struct
**          point of operate function
** OUTPUT:
**          null
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhSCTPDataRecv(IuhMultiHomedSocket *sockPtr, void (*IuhManageIncomingPacket) (struct IuhMultiHnbSocket *, char *, int, int)) {
	fd_set fset;
	struct timeval timeout;
	int max = 0, i = 0, j = 0;
	struct IuhMultiHomedInterface *inf;	
	struct IuhMultiHnbSocket *subinf;
	//struct sctp_sndrcvinfo sinfo;
	char buf[2048];
	int readBytes;
	int Data_type = 0;
	
	if(sockPtr == NULL || sockPtr->count == 0 || IuhManageIncomingPacket == NULL) return Iuh_FALSE;
	
	FD_ZERO(&fset);
	inf = sockPtr->interfaces;
	// select() on all the sockets
	//printf("sockPtr->count1 = %d\n",sockPtr->count);
	for(i = 0; i < sockPtr->count; i++) {
		FD_SET(inf->sock, &fset);
		if(inf->sock > max) max = inf->sock;
		subinf = inf->sub_sock;
		for(j = 0; j < inf->sub_sock_count; j++){
			FD_SET(subinf->sock, &fset);			
			if(subinf->sock > max) max = subinf->sock;
			if(subinf->sock_next == NULL)
				break;
			subinf = subinf->sock_next;
		}
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
	}
	//printf("max = %d\n",max);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	while(select(max+1, &fset, NULL, NULL, &timeout) < 0) {
		if(errno != EINTR) {
            iuh_syslog_debug_debug(IUH_DEFAULT,"@@@select loop error!@@@\n");
		}
	}
	inf = sockPtr->interfaces;
	subinf = NULL;
	for(i = 0; i < sockPtr->count; i++) {
		if(FD_ISSET(inf->sock, &fset)) {
			IuhSctpAccept(inf);
		}else{
			subinf = inf->sub_sock;
			if(subinf != NULL){
    			for(j = 0; j < inf->sub_sock_count; j++){
    				if(FD_ISSET(subinf->sock, &fset)) {
    				    memset(buf,0,2048);
    				    readBytes = 0;
						subinf->sinfo.sinfo_ppid = 0; //book add, 2011-12-20
    				    int ret = IuhSCTPReceive(subinf->sock,buf,2047,&subinf->sinfo,&readBytes,(struct sockaddr*)&subinf->addr);
    					if(ret){
    					    iuh_syslog_debug_debug(IUH_DEFAULT,"subinf->sinfo.sinfo_ppid = %d\n",subinf->sinfo.sinfo_ppid);
    						Data_type = IuhDataTypeCheck(subinf->sinfo.sinfo_ppid);
    						if(Data_type){
    							IuhManageIncomingPacket(subinf, buf, readBytes, Data_type);
    						}
    					}
    					else if(ret == 0) {
    					    // we can delete hnb by sock here
    					    IUH_DELETE_HNB(subinf->HNBID);
    					    Delete_Hnb_Socket_by_sock(subinf->sock,inf->sock);
    					}
    				}
    				if(subinf->sock_next == NULL)
    					break;
    				subinf = subinf->sock_next;
    			}
			}
		}
		
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
	}

	
	return Iuh_TRUE;
}

