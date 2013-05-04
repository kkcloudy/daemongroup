#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include "iuh/Iuh.h"
#include "Iuh_Decode.h"
#include "Iuh_Encode.h"
#include "Iuh_Thread.h"
#include "Iuh_SockOP.h"
#include "Iuh_DBus.h"
#include "Iuh_IuRecv.h"
#include "Iuh_ManageHNB.h"
#include "Iuh_log.h"
#include "Iuh_DBus_handler.h"
#include "RANAP-PDU.h"


/*****************************************************
** DISCRIPTION:
**          Receive data from gIuhSockets
** INPUT:
**          void
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void FemtoIuhRun(){
    iuh_syslog_debug_debug(IUH_DEFAULT,"&&&&&&&&&&&&&Call FemtoIuhRun\n");
	while(1){
		if((IuhSCTPDataRecv(&gIuhSocket, IuhManageIncomingPacket)) == Iuh_FALSE){   // CWACManageIncomingPacket will be called	
			sleep(2);
		}
	}
}


/*****************************************************
** DISCRIPTION:
**          Check whether HnbId is legal
** INPUT:
**          HnbId
** OUTPUT:
**          null
** RETURN:
**          true
**          false
*****************************************************/

IuhBool check_hnbid_func(unsigned int HNBID){
	if(HNBID > HNB_DEFAULT_NUM_AUTELAN){
		iuh_syslog_err("<error> invalid HNBid:%d\n",HNBID);
		return Iuh_FALSE;
	}else{
		return Iuh_TRUE;
	}
}


/*****************************************************
** DISCRIPTION:
**          This function print the content of buffer
** INPUT:
**          buffer size
**          buffer
** OUTPUT:
**          Message data
** RETURN:
**          void
*****************************************************/

void CWCapture(int n ,unsigned char *buffer){
		int t=0;
		while((n-t)>=16)
		{
			int i;
			printf("[");
			for(i=0;i<16;i++)
				printf("%02x ",buffer[t+i]);
			printf("]\t[");
			for(i=0;i<16;i++)
			{
				char ch=buffer[t+i];
				if(isalnum(ch))
					printf("%c",ch);
				else
					printf(".");
			}
			printf("]\n");
			t+=16;
		}

		if(n>t)
		{
			int i=t;
			printf("[");
			while(i<n)
				printf("%02x ",buffer[i++]);
			printf("]");
			i=n-t;
			i=16-i;
			while(i--)
				printf("   ");
			printf("\t[");
			i=t;
			while(i<n)
			{
				char ch=buffer[i++];
				if(isalnum(ch))
					printf("%c",ch);
				else
					printf(".");
			}
			printf("]\n");
		}
		printf("\n\n");
}


/*****************************************************
** DISCRIPTION:
**          Parse Hnb Register Request message
** INPUT:
**          buf
**          buf size
** OUTPUT:
**          hnb register request struct
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseHnbRegister(const char *buf, const int readBytes, HnbRegisterRequestValue *requestValue, HNBAPCause *cause)
{
    if(!IuhDecodeHnbRegisterRequest(buf, readBytes, requestValue, cause)){
        iuh_syslog_err("decode HNB Register Request error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Parse Hnb De-Register Request message
** INPUT:
**          buf
**          buf size
** OUTPUT:
**          reject cause if exists
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseHnbDeregister(const char *buf, const int readBytes, HNBAPCause *cause)
{
    if(!IuhDecodeHnbDeregister(buf, readBytes, cause)){
        iuh_syslog_err("decode HNB DeRegister error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          parse ue register request message
** INPUT:
**          buf
**          buf size
** OUTPUT:
**          ue register request struct
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseUERegister(const char *buf, const int readBytes, UERegisterRequestValue *UERequestValue, HNBAPCause *cause)
{
    if(!IuhDecodeUERegisterRequest(buf, readBytes, UERequestValue, cause)){
        iuh_syslog_err("decode UE Register Request error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Parse ue De-Register request message
** INPUT:
**          buf
**          buf size
** OUTPUT:
**          reject cause if exists
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseUEDeregister(const char *buf, const int readBytes, HNBAPCause *cause, char *context_id)
{
    if(!IuhDecodeUEDeregister(buf, readBytes, cause, context_id)){
        iuh_syslog_err("decode UE DeRegister error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Parse Rua onnnect message 
** INPUT:
**          buf
**          buf size
** OUTPUT:
**          Iuh2IuMsg struct
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseConnect(const char *buf, const int readBytes, Iuh2IuMsg *sigMsg)
{
    if(!IuhDecodeConnect(buf, readBytes, sigMsg)){
        iuh_syslog_err("decode RUA Connect error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Parse Rua direct transfer message
** INPUT:
**          buf
**          buf size
** OUTPUT:
**          Iuh2IuMsg struct
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseDirectTransfer(const char *buf, const int readBytes, Iuh2IuMsg *sigMsg)
{
    if(!IuhDecodeDirectTransfer(buf, readBytes, sigMsg)){
        iuh_syslog_err("decode RUA Direct Transfer error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Parse Rua disconnect message
** INPUT:
**          buf
**          buf size
** OUTPUT:
**          Iuh2IuMsg struct
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseDisconnect(const char *buf, const int readBytes, Iuh2IuMsg *sigMsg)
{
    if(!IuhDecodeDisconnect(buf, readBytes, sigMsg)){
        iuh_syslog_err("decode RUA Disconnect error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Parse Rua connectionless transfer message
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          Iuh2IuMsg struct
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhParseConnectionlessTransfer(const char *buf, const int readBytes, Iuh2IuMsg *sigMsg)
{
    if(!IuhDecodeConnectionlessTransfer(buf, readBytes, sigMsg)){
        iuh_syslog_err("decode RUA COnnnectionless Transfer error\n");
        return Iuh_FALSE;
    }

    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          iuh send message to iu
** INPUT:
**          Iuh2IuMsg struct
**          socket queue id
** OUTPUT:
**          null
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhSendMsg2Iu(Iuh2IuMsg *sigMsg, int QID){
    int ret;
    iuh_syslog_debug_debug(IUH_DEFAULT,"send msg to iu QID = %d\n",QID);
    iuh_syslog_debug_debug(IUH_DEFAULT,"sigMsg len = %d\n",sizeof(Iuh2IuMsg));
	iuh_syslog_debug_debug(IUH_DEFAULT,"\nIUH2IU MESSAGE :\n");
    iuh_syslog_debug_debug(IUH_DEFAULT,"---RncID = %d\n",sigMsg->RncPlmn.RncId);
	iuh_hnb_show_str("---PlmnID = ", sigMsg->RncPlmn.PlmnId, 1, PLMN_LEN);
    iuh_syslog_debug_debug(IUH_DEFAULT,"---SigType = %d\n",sigMsg->SigType);
    iuh_syslog_debug_debug(IUH_DEFAULT,"---CnDomainID = %d\n",sigMsg->CnDomain);
    iuh_syslog_debug_debug(IUH_DEFAULT,"---EstabCause = %d\n",sigMsg->EstabCause);
	iuh_hnb_show_str("---IMSI = ", sigMsg->UeIdentity.choice.IMSI, 1, IMSI_LEN);
    iuh_syslog_debug_debug(IUH_DEFAULT,"---RuaType = %d\n",sigMsg->RuaType.ruaType);
    iuh_syslog_debug_debug(IUH_DEFAULT,"---RanapMsg.size = %d\n",sigMsg->RanapMsg.size);
	iuh_hnb_show_str("---ContextID = ", sigMsg->contextid, 0, CONTEXTID_LEN);
	iuh_syslog_debug_debug(IUH_DEFAULT,"---UEID = %d\n",sigMsg->UEID);
	iuh_syslog_debug_debug(IUH_DEFAULT,"---ueop = %d\n",sigMsg->ueop);
		
    if(QID > 16) return Iuh_FALSE;
    //CWCapture(sizeof(Iuh2IuMsg),sigMsg);
//    ret = sendto(sockPerThread[QID-1], sigMsg, sizeof(Iuh2IuMsg), 0, (struct sockaddr*)(&toIU.addr), toIU.addrlen);
	ret = sendto(TipcsockPerThread[QID-1], sigMsg, sizeof(Iuh2IuMsg), 0, (struct sockaddr*)(&Iuh2Iu_tipcaddr), sizeof(Iuh2Iu_tipcaddr));
    if(ret == -1){
        perror("send error");
        return Iuh_FALSE;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"sizeof(Iuh2IuMsg) = %d\n",sizeof(Iuh2IuMsg));
    iuh_syslog_debug_debug(IUH_DEFAULT,"ret = %d\n",ret);

	iuh_syslog_info("###  Send Message To Iu Interface \n");
	iuh_syslog_info("#################################\n\n\n");
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Send message to hnb
** INPUT:
**          HnbId
**          message to send
**          ppid
** OUTPUT:
**          null
** RETURN:
**          true
**          false
*****************************************************/

IuhBool IuhSendMessage(int HNBID, IuhProcotolMsg *msgPtr, int ppid){
    struct IuhMultiHnbSocket *sock_info = IUH_FIND_HNB_ID(HNBID);
    iuh_syslog_debug_debug(IUH_DEFAULT,"HNBID = %d\n", HNBID);
    if(sock_info == NULL){
        return Iuh_FALSE;
    }

	sock_info->sinfo.sinfo_ppid = ppid;

    if(!IuhSCTPSend(sock_info, msgPtr)) {
		iuh_syslog_debug_debug(IUH_DEFAULT,"Error: IuhSCTPSend in IuhSendMessage err.\n");
        return Iuh_FALSE;
    }
    
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          This function receive get messages from message queue
**      and deal with these messages. Message type can be HNBAP,
**      RUA and message from iu.
** INPUT:
**          void
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void * IuhManageHNB(void *arg) {
	int QID = ((IuhThreadArg*)arg)->index;
	int HNBMsgqID;
	int wait = 1;
	msgq HNBMsgq;
	IUH_FREE_OBJECT(arg);
	IuhGetMsgQueue(&HNBMsgqID);
	while(1){
	    int HNBID;
	    int i = 0;
        int UEID = 0;
		while (wait){
			memset((char*)&HNBMsgq, 0, sizeof(HNBMsgq));
			if (msgrcv(HNBMsgqID, (msgq*)&HNBMsgq, sizeof(HNBMsgq.mqinfo), QID, 0) == -1) {
				perror("msgrcv");
				continue;
				//exit(1);
			}
            iuh_syslog_debug_debug(IUH_DEFAULT,"receive message\n");
            iuh_syslog_debug_debug(IUH_DEFAULT,"message type = %d\n",HNBMsgq.mqinfo.type);
			if(HNBMsgq.mqinfo.type == HNBAP_DATA){
			    iuh_syslog_debug_debug(IUH_DEFAULT,"receive hnbap message\n");
				HNBID = HNBMsgq.mqinfo.HNBID;
				if(!check_hnbid_func(HNBID)){
					continue;
				}else{
				}
				if(IUH_HNB[HNBID] == NULL){
				    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_HNB[%d] == NULL\n",HNBID);
					continue;
				}
				break;
			}
			else if(HNBMsgq.mqinfo.type == RUA_DATA){
			    iuh_syslog_debug_debug(IUH_DEFAULT,"receive rua message\n");
			    HNBID = HNBMsgq.mqinfo.HNBID;
				if(!check_hnbid_func(HNBID)){
					continue;
				}else{
				}
				if(IUH_HNB[HNBID] == NULL){
				    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_HNB[%d] == NULL\n",HNBID);
					continue;
				}
			    break;
			}
			else if(HNBMsgq.mqinfo.type == IU_DATA){
			    iuh_syslog_debug_debug(IUH_DEFAULT,"receive iu message\n");
			    HNBID = HNBMsgq.mqinfo.HNBID;
			    
			    if(HNBMsgq.mqinfo.subtype == CONNECTIONLESS) break;
			    
				if(!check_hnbid_func(HNBID)){
					continue;
				}else{
				}
				if(IUH_HNB[HNBID] == NULL){
				    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_HNB[%d] == NULL\n",HNBID);
					continue;
				}
			    break;
			}
			/* book add,2011-12-21 */
			else if(HNBMsgq.mqinfo.type == CONTROL_DATA){
				iuh_syslog_debug_debug(IUH_DEFAULT,"receive control message\n");
			    HNBID = HNBMsgq.mqinfo.HNBID;
				if(!check_hnbid_func(HNBID)){
					continue;
				}else{
				}
				if(IUH_HNB[HNBID] == NULL){
				    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_HNB[%d] == NULL\n",HNBID);
					continue;
				}
				break;
			}
			else {
			    iuh_syslog_err("something wrong\n");
				continue;
			}
		}
		
        if(HNBMsgq.mqinfo.type == HNBAP_DATA) {
            HNBAP_PDU_t hnbap_pdu;
            HNBAPCause cause;
            int errflag = 0;
            if(!IuhParseHnbapMessage(HNBMsgq.mqinfo.u.DataInfo.Data, HNBMsgq.mqinfo.u.DataInfo.len, &hnbap_pdu)){
		        perror("parse HNBAP-PDU error");
		        cause.present = Cause_protocol;
		        cause.choice.protocol = Protocol_abstract_syntax_error_reject;
		        errflag = 1;
		    }
            else{
                if(hnbap_pdu.present != HNBAP_PDU_PR_initiatingMessage){
                    iuh_syslog_err("unknow message\n");
                    cause.present = Cause_protocol;
                    cause.choice.protocol = Protocol_message_not_compatible_with_receiver_state;
                    errflag = 1;
		            continue;
                }
                else{
                    int ret;   
                    IuhProcotolMsg msgPtr;
                    InitiatingMessage_t	 *initiatingMessage = &(hnbap_pdu.choice.initiatingMessage);
                    iuh_syslog_debug_debug(IUH_DEFAULT,"initiatingMessage->procedureCode = %d\n",initiatingMessage->procedureCode);
                    switch(initiatingMessage->procedureCode) {
                        case HNBRegister: {
        		            HnbRegisterRequestValue requestValue;
        		            if(!IuhParseHnbRegister((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &requestValue, &cause)){
        		                iuh_syslog_err("Error: decode HNB Register error\n");
        		                IUH_HNB[HNBID]->state = UNREGISTED;
        		                cause.present = Cause_protocol;
		                        cause.choice.protocol = Protocol_abstract_syntax_error_reject;
        		                errflag = 1;
        		            }
        		            else{
        		                ret = IUH_UPDATE_HNB(HNBID, &requestValue);
        		                
        		                if(ret != 0){
        		                    iuh_syslog_err("Error: update Hnb info %d err\n",ret);
        		                }
        		                /* accept register */
        		                if(!IuhAssembleRegisterAccept(HNBID, &msgPtr)){
        		                    iuh_syslog_err("Error: assemble Register Accept error\n");
        		                    cause.present = Cause_protocol;
		                            cause.choice.protocol = Protocol_abstract_syntax_error_falsely_constructed_message;
        		                    errflag = 1;
        		                    break;
        		                }
        		                /* send message */
            		            if(!IuhSendMessage(HNBID, &msgPtr, HNBAP_PPID)) {
            		                iuh_syslog_err("Error: send HNB Register Accept error  HNBID = %d\n",HNBID);
            		            }
        		            }
                            break;
                        }
                        case HNBDeRegister:{
                            iuh_syslog_debug_debug(IUH_DEFAULT,"HNB deregister \n");
                            IUH_HNB[HNBID]->state = UNREGISTED;
                            if(!IuhParseHnbDeregister((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &cause)){
                                iuh_syslog_err("decode HNB DeRegister error\n");
                                IUH_HNB[HNBID]->cause = cause;
                                cause.present = Cause_protocol;
	                            cause.choice.protocol = Protocol_abstract_syntax_error_reject;
    		                    errflag = 1;
                                break;
                            }
                            else{
                                iuh_syslog_debug_debug(IUH_DEFAULT,"IuhParseHnbDeregister ok \n");
                            }
                            
                            struct IuhMultiHnbSocket *sock_info = IUH_FIND_HNB_ID(HNBID);
                            if((sock_info != NULL) && (sock_info->if_belong != NULL)){
                                iuh_syslog_debug_debug(IUH_DEFAULT,"sock_info->sock = %d,  sock_info->if_belong->sock = %d \n",sock_info->sock, sock_info->if_belong->sock);
                                //Delete_Hnb_Socket_by_sock(sock_info->sock, sock_info->if_belong->sock);
                                IUH_BZERO_HNBID_BY_SOCK(sock_info->sock, sock_info->if_belong->sock);
                                //iuh_syslog_debug_debug(IUH_DEFAULT,"delete hnb sock from gIuhSocket OK\N");
                                IUH_DELETE_HNB(HNBID);
                            }
                            else{
                                iuh_syslog_debug_debug(IUH_DEFAULT,"sock is NULL\n");
                            }
                            break;
                        }
                        case UERegister:{
                            iuh_syslog_debug_debug(IUH_DEFAULT,"UERegister\n");
                            UERegisterRequestValue UERequestValue;
        		            if(!IuhParseUERegister((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &UERequestValue, &cause)){
        		                iuh_syslog_err("decode UE Register error\n");
        		                cause.present = Cause_protocol;
	                            cause.choice.protocol = Protocol_abstract_syntax_error_reject;
    		                    errflag = 1;
        		               
        		            }
        		            else{
								int retval = IMSI_EXIST_IN_LIST;
								#ifdef IMSI_WHITE_LIST_SWITCH			
								iuh_syslog_debug_debug(IUH_DEFAULT,"IMSI_WL_Switch=%d\n", IUH_HNB[HNBID]->IMSI_WL_Switch);
								if(IUH_HNB[HNBID] && IUH_HNB[HNBID]->IMSI_WL_Switch)
								{
									IMSIWHITELIST* head = IUH_HNB[HNBID]->imsi_white_list;
									retval = IUH_FIND_IMSI_IN_ACL_WHITE_LIST(head, UERequestValue.IMSI);
									iuh_syslog_debug_debug(IUH_DEFAULT,"retval=%d\n", retval);
								}
								iuh_syslog_debug_debug(IUH_DEFAULT,"after retval=%d\n", retval);
								#endif								
								if(retval == IMSI_EXIST_IN_LIST){
									char tmpIMSI[8] = {0};
	        		                if(memcmp(UERequestValue.IMSI,tmpIMSI,8) != 0){
	        		                    UEID = IUH_FIND_UE_BY_IMSI(UERequestValue.IMSI);
										iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d\n",UEID);
	        		                }
	        		                
	                                if((UEID != 0) && (IUH_HNB_UE[UEID]->HNBID == HNBID)){
	                                    IUH_UPDATE_UE(UEID, HNBID, &UERequestValue);
	                                }
	                                else{
	                                    if(UEID != 0){
	                                        IUH_CMD_DELETE_UE(UEID);
	                                    }
	                                
	            		                for(i = 1; i <= UE_MAX_NUM_AUTELAN; i++) {	
	            							if(IUH_HNB_UE[i] == NULL){
	            								UEID = i;
	            								break;
	            							}
	            						}
	            		                if(UEID == 0){
	                                        /* if this is an emergency call, we need to provide service for this UE */
	            		                
	            		                    /* there is no free ue to use */
											iuh_syslog_debug_debug(IUH_DEFAULT,"there is no free ue to use\n");
	        		                
	            		                    cause.present = Cause_misc;
	            		                    cause.choice.misc = Misc_processing_overload;
	            		                    if(!IuhAssembleUERegisterReject(UEID, &cause, &msgPtr)){
	                		                    iuh_syslog_err("assemble UE Register Reject error\n");
	                		                    cause.present = Cause_protocol;
		                                        cause.choice.protocol = Protocol_abstract_syntax_error_falsely_constructed_message;
	                		                    errflag = 1;
	                		                }
	            		                }
	            		                ret = IUH_CREATE_UE(HNBID, UEID, &UERequestValue);
	            		                
	            		                if(ret != 0){
	            		                    iuh_syslog_err("Error: Update UE info %d\n",ret);
	            		                }
	            		            }
									iuh_syslog_debug_debug(IUH_DEFAULT,"end else, HNBID = %d, UEID = %d\n",HNBID,UEID);
	        		                /* accept register */
	        		                if(!IuhAssembleUERegisterAccept(UEID,&msgPtr)){
	        		                    iuh_syslog_err("Error: assemble UE Register Accept error\n");
	        		                    cause.present = Cause_protocol;
			                            cause.choice.protocol = Protocol_abstract_syntax_error_reject;
	        		                    errflag = 1;
	        		                    break;
	        		                }
	        		                /* send message */
	            		            if(!IuhSendMessage(HNBID, &msgPtr,HNBAP_PPID)) {
	            		                iuh_syslog_err("Error: send UE Register Accept error\n");
	            		            }
								}
        		                
        		            }
                            break;
                        }
                        case UEDeRegister: {
                            char ContextID[CONTEXTID_LEN] = {0};
                            if(!IuhParseUEDeregister((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &cause, ContextID)){
                                iuh_syslog_err("Error: decode UE DeRegister error\n");
                                cause.present = Cause_protocol;
	                            cause.choice.protocol = Protocol_abstract_syntax_error_falsely_constructed_message;
    		                    errflag = 1;
                            }
                            else{
                                UEID = FINDUEID(HNBID, ContextID);
                            }
                            
                            ret = IUH_DELETE_UE(UEID);
                            if(ret != 0){
    		                    iuh_syslog_err("Error: Error Delete UE info %d\n",ret);
    		                }
                            break;
                        }
                        case ProcErrorIndication: {
                            if(!IuhDecodeHNBAPErrorIndication((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &cause)){
                                iuh_syslog_err("Error: decode HNBAP Error indication error\n");
                                cause.present = Cause_protocol;
	                            cause.choice.protocol = Protocol_abstract_syntax_error_ignore_and_notify;
    		                    errflag = 1;
                            }
                            else{
                                iuh_syslog_debug_debug(IUH_DEFAULT, "Receive HNBAP ErrorIndication Type: %d\n", cause.present);
                                switch(cause.present){
                                    case Cause_radioNetwork :{
                                        iuh_syslog_debug_debug(IUH_DEFAULT,"HNBAP ErrorIndication : %d\n", cause.choice.radioNetwork);
                                        break;
                                    }
                                    case Cause_transport :{
                                        iuh_syslog_debug_debug(IUH_DEFAULT,"HNBAP ErrorIndication : %d\n", cause.choice.transport);
                                        break;
                                    }
                                    case Cause_protocol :{
                                        iuh_syslog_debug_debug(IUH_DEFAULT,"HNBAP ErrorIndication : %d\n", cause.choice.protocol);
                                        break;
                                    }
                                    case Cause_misc :{
                                        iuh_syslog_debug_debug(IUH_DEFAULT,"HNBAP ErrorIndication : %d\n", cause.choice.misc);
                                        break;
                                    }
                                    default:
                                        break;
                                }
                            }
                            break;
                        }
                        case privateMessage: {
                            break;
                        }
                        case CSGMembership: {
                            break;
                        }
                        default:
                            break;
                    }
                    
                }
            }
            if(errflag == 1){
                IuhHnbapErrorIndication(&cause, HNBID);
            }
        }
        else if(HNBMsgq.mqinfo.type == RUA_DATA){
            iuh_syslog_debug_debug(IUH_DEFAULT,"RUA_DATA\n");
            RUA_PDU_t rua_pdu;
            Iuh2IuMsg sigMsg;
		    RuaCause cause;
		    int send2iuflag = 0;
			RANAP_PDU_t ranap_pdu;
			IuhProcotolMsg msgPtr;
			InitiatingMessage_t	 *initiatingMessage = NULL;
			int errflag = 0;
            if(!IuhParseRuaMessage(HNBMsgq.mqinfo.u.DataInfo.Data, HNBMsgq.mqinfo.u.DataInfo.len, &rua_pdu)){
		        perror("parse RUA-PDU error");
		        sigMsg.cause.present = Cause_protocol;
                sigMsg.cause.choice.protocol = Protocol_abstract_syntax_error_reject;
                send2iuflag = 1;
		    }
		    else{
		        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhParseRuaMessage\n"); 
		        
		        memset(&sigMsg, 0, sizeof(sigMsg));
                initiatingMessage = &(rua_pdu.choice.initiatingMessage);
                iuh_syslog_debug_debug(IUH_DEFAULT,"initiatingMessage->procedureCode = %d\n",initiatingMessage->procedureCode);
                
                switch(initiatingMessage->procedureCode) {
                    case Pro_Connect: {
                        sigMsg.RuaType.ruaType = Pro_Connect;
                        sigMsg.SigType = CONNECTION_TO_CN;
                        if(!IuhParseConnect((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &sigMsg)){
    		                iuh_syslog_err("decode RUA Connect error\n");
    		                sigMsg.cause.present = Cause_protocol;
                            sigMsg.cause.choice.protocol = Protocol_abstract_syntax_error_reject;
    		                send2iuflag = 1;
    		            }
    		            else{
							/* parse initial-ue-message and get informations, book add 2011-12-16 */
							UEID = IUH_FIND_UE_BY_CTXID(sigMsg.contextid);
							if(!IuhDecodeRanapMessage(sigMsg.RanapMsg.RanapMsg, sigMsg.RanapMsg.size, &ranap_pdu)){
						        iuh_syslog_err("Error: Iuh decode RANAP PDU error\n");
						    }
							if(ranap_pdu.present == RANAP_PDU_PR_initiatingMessage) {
								initiatingMessage = &(ranap_pdu.choice.initiatingMessage);
								switch(initiatingMessage->procedureCode){
									case Pro_id_InitialUE_Message: {
									#ifdef IU_SIG_CON_ID
										if(!IuhDecodeRanapInitialUEMessage(initiatingMessage->value.buf, initiatingMessage->value.size, UEID)){
											iuh_syslog_err("Error: Iuh decode RANAP initial ue message error\n");
										}
									#endif
										break;
									}
									default:
										break;
								}
							}
    		            }
                        break;
                    }
                    case Pro_DirectTransfer: {
                        sigMsg.RuaType.ruaType = Pro_DirectTransfer;
                        sigMsg.SigType = CONNECTION_TO_CN;
                        if(!IuhParseDirectTransfer((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &sigMsg)){
    		                iuh_syslog_err("decode RUA DirectTransfer error\n");
    		                sigMsg.cause.present = Cause_protocol;
                            sigMsg.cause.choice.protocol = Protocol_abstract_syntax_error_reject;
    		                send2iuflag = 1;
    		            }
    		            else{
    		            }
                        break;
                    }
                    case Pro_Disconnect: {
                        sigMsg.RuaType.ruaType = Pro_Disconnect;
                        sigMsg.SigType = CONNECTION_TO_CN;
                        if(!IuhParseDisconnect((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &sigMsg)){
    		                iuh_syslog_err("decode RUA Disconnect error\n");
    		                sigMsg.cause.present = Cause_protocol;
                            sigMsg.cause.choice.protocol = Protocol_abstract_syntax_error_reject;
    		                send2iuflag = 1;
    		            }
    		            else{
    		            }
                        break;
                    }
                    case Pro_ConnectionlessTransfer: {
                        sigMsg.RuaType.ruaType = Pro_ConnectionlessTransfer;
                        sigMsg.SigType = CONNECTIONLESS_TO_CN;
                        if(!IuhParseConnectionlessTransfer((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &sigMsg)){
    		                iuh_syslog_err("decode RUA ConnectionlessTransfer error\n");
    		                sigMsg.cause.present = Cause_protocol;
                            sigMsg.cause.choice.protocol = Protocol_abstract_syntax_error_reject;
                            send2iuflag = 1;
    		            }
    		            else{
						
							/* book add for HNB init Reset, 2011-12-15 */
						    if(!IuhDecodeRanapMessage(sigMsg.RanapMsg.RanapMsg, sigMsg.RanapMsg.size, &ranap_pdu)){
						        iuh_syslog_err("Error: Iuh decode RANAP message error\n");
						    }
							if(ranap_pdu.present == RANAP_PDU_PR_initiatingMessage)
							{
								initiatingMessage = &(ranap_pdu.choice.initiatingMessage);
								switch(initiatingMessage->procedureCode){
									case Pro_id_Reset:
									{
									#ifdef HNB_INIT_RESET
										if(!IuhAssembleRanapResetAck(&sigMsg)){
											iuh_syslog_err("Error: Assemble Ranap Reset ACK error\n");
				                            errflag = 1;
										}
										if(!IuhAssembleConnectionlessTransfer(&sigMsg, &msgPtr)){
				                            iuh_syslog_err("Error: Assemble RUA ConnectionlessTransfer error\n");
				                            errflag = 1;
				                        }
										if((errflag==0) && (!IuhSendMessage(HNBID, &msgPtr, RUA_PPID))) {
							                iuh_syslog_err("Error: send RUA message error. Affact HNBID = %d\n",HNBID);
											errflag = 1;
							            }

										if(IUH_HNB[HNBID] != NULL)
										{
											Iuh_HNB_UE *tmpUE = NULL;
											int flag = 0;
											tmpUE = IUH_HNB[HNBID]->HNB_UE;
											for(i = 0; i < IUH_HNB[HNBID]->current_UE_number; i++,tmpUE=tmpUE->ue_next)
											{
												if(NULL != tmpUE)
												{
													if((sigMsg.CnDomain==CS_Domain) && (memcmp(tmpUE->iuCSSigConId,tmpUE->iuDefSigConId,IU_SIG_CONN_ID_LEN)!=0))
														flag = 1;
													else if((sigMsg.CnDomain==PS_Domain) && (memcmp(tmpUE->iuPSSigConId,tmpUE->iuDefSigConId,IU_SIG_CONN_ID_LEN)!=0))
														flag = 1;
												}
											}
											iuh_syslog_debug_debug(IUH_DEFAULT, "flag = %d\n", flag);
											/* send Reset Resource with all iu-conn-list to iu, then to cn */
											if((0==errflag) && (1==flag)){
												iuh_syslog_debug_debug(IUH_DEFAULT,"HNBID = %d\n",HNBID);
												sigMsg.RuaType.msgType = RUA_PR_initiatingMessage;
								                sigMsg.RncPlmn.RncId = IUH_HNB[HNBID]->rncId;
								                memcpy(sigMsg.RncPlmn.PlmnId, IUH_HNB[HNBID]->plmnId, PLMN_LEN);
												/* send msg to cn */
												if(!IuhAssembleRanapResetResource(&msgPtr, HNBID, &sigMsg)){
													iuh_syslog_err("Error: Assemble Ranap Reset Resource error\n");
												}
												else{
													if(!IuhSendMsg2Iu(&sigMsg,QID)){
									                    iuh_syslog_err("Error: Iuh send message to Iu error\n");
									                }
												}
											}
										}
									#endif
										send2iuflag = 2;//do nothing
										break;
									}
									default:
										break;
								}
							}
    		            }
                        break;
                    }
                    case Pro_ErrorIndication: {
                        sigMsg.RuaType.ruaType = Pro_ErrorIndication;
                        if(!IuhDecodeRUAErrorIndication((char*)initiatingMessage->value.buf, initiatingMessage->value.size, &cause)){
                            iuh_syslog_err("Error: decode RUA Error Indication error\n");
                            sigMsg.cause.present = Cause_protocol;
                            sigMsg.cause.choice.protocol = Protocol_abstract_syntax_error_ignore_and_notify;
                            send2iuflag = 1;
                        }
                        else{
                            iuh_syslog_debug_debug(IUH_DEFAULT, "Receive RUA ErrorIndication Type: %d\n", cause.present);
                            switch(cause.present){
                                case Cause_radioNetwork :{
                                    iuh_syslog_debug_debug(IUH_DEFAULT, "RUA ErrorIndication : %d\n", cause.choice.radioNetwork);
                                    break;
                                }
                                case Cause_transport :{
                                    iuh_syslog_debug_debug(IUH_DEFAULT, "RUA ErrorIndication : %d\n", cause.choice.transport);
                                    break;
                                }
                                case Cause_protocol :{
                                    iuh_syslog_debug_debug(IUH_DEFAULT, "RUA ErrorIndication : %d\n", cause.choice.protocol);
                                    break;
                                }
                                case Cause_misc :{
                                    iuh_syslog_debug_debug(IUH_DEFAULT, "RUA ErrorIndication : %d\n", cause.choice.misc);
                                    break;
                                }
                                default:
                                    break;
                            }
                        }
                        break;
                    }
                    case Pro_privateMessage: {
                        sigMsg.RuaType.ruaType = Pro_privateMessage;
                        break;
                    }
                    default:
                        break;
                }
		    }
		    if(0 == send2iuflag){
                sigMsg.RuaType.msgType = RUA_PR_initiatingMessage;
                sigMsg.RncPlmn.RncId = IUH_HNB[HNBID]->rncId;
                memcpy(sigMsg.RncPlmn.PlmnId, IUH_HNB[HNBID]->plmnId, PLMN_LEN);
                
                if(!IuhSendMsg2Iu(&sigMsg,QID)){
                  /* 
                              ** if can not send message to iu,
                              ** we should give hnb an error indication within RUA message
                              */
                    iuh_syslog_err("Error: Iuh send message to Iu error\n");
                    sigMsg.cause.present = Cause_transport;// book modify, 2011-12-06
                    sigMsg.cause.choice.transport = Transport_transport_resource_unavailable;
                    IuhRuaErrorIndication(&sigMsg,HNBID);
                }
            }
            else if(1 == send2iuflag){
                IuhRuaErrorIndication(&sigMsg,HNBID);
            }
        }
        else if(HNBMsgq.mqinfo.type == IU_DATA){// IU_DATA
            
            IuhProcotolMsg msgPtr;
            int errflag = 0;
			unsigned int iu_HNBID = 0;
			unsigned int iu_UEID = 0;
            Iuh2IuMsg *sigMsg = (Iuh2IuMsg *)HNBMsgq.mqinfo.u.DataInfo.Data;
			CWCapture(sigMsg->RanapMsg.size ,(unsigned char*)sigMsg->RanapMsg.RanapMsg);

			/* Decode Ranap Message */
			RANAP_PDU_t ranap_pdu;
            if(!IuhDecodeRanapMessage(sigMsg->RanapMsg.RanapMsg, sigMsg->RanapMsg.size, &ranap_pdu)){
                iuh_syslog_err("Error: Iuh decode RANAP message error\n");
            }
			else if(ranap_pdu.present == RANAP_PDU_PR_initiatingMessage){
				InitiatingMessage_t  *initiatingMessage = &(ranap_pdu.choice.initiatingMessage);
				UEID = IUH_FIND_UE_BY_CTXID(sigMsg->contextid);
            	
				switch(sigMsg->ranap_type){
					case id_RAB_Assignment:{
					#ifdef RAB_INFO
						if((UEID > 0) && (UEID < UE_MAX_NUM_AUTELAN)){
							if(!Iuh_decode_RAB_Assignment(initiatingMessage->value.buf, initiatingMessage->value.size, UEID)){
								iuh_syslog_err("Error: Iuh decode RAB Assignment error\n");
							}
						}
					#endif
		                break;
		            }
		            case id_RAB_ReleaseRequest:{
					#ifdef RAB_INFO
					    if((UEID > 0) && (UEID < UE_MAX_NUM_AUTELAN)){
    	                	if(!Iuh_decode_RAB_Release(initiatingMessage->value.buf, initiatingMessage->value.size, UEID)){
    							iuh_syslog_err("Error: Iuh decode RAB Release Request error\n");
    	                	}
	                	}
		                break;
					#endif
		            }
					case id_RelocationResourceAllocation:{
					#ifdef INBOUND_HANDOVER //book add, 2011-12-17
					    iu_UEID = sigMsg->UEID;
					    if(IUH_HNB_UE[iu_UEID] == NULL)
					    {
    						char tmp_CellID[CELLID_LEN] = {0};
    						UERegisterRequestValue tmp_ueInfo;
    						if(!Iuh_decode_Relocation_Request(initiatingMessage->value.buf, initiatingMessage->value.size, tmp_CellID, &sigMsg->CnDomain))
    						{
    							iuh_syslog_err("Iuh decode Relocation Request error\n");
    						}
    						else
    						{
    						    int i = 0;
    						    int ret = 0;
    					        memcpy(tmp_ueInfo.IMSI, sigMsg->imsi, IMSI_LEN);
    					        /* find hnb */
    					        iu_HNBID = IUH_FIND_HNB_CellId(tmp_CellID);
        						/* create new ue */
    					        ret = IUH_CREATE_UE(iu_HNBID, iu_UEID, &tmp_ueInfo);
        		                if(ret != 0)
        		                {
        		                    iuh_syslog_err("Error: create UE%d by Relocation Request failed!!!\n",iu_UEID);
        		                    errflag = 1;
        		                }
        		                HNBID = iu_HNBID;
                                sigMsg->EstabCause = NORMAL; // maybe emergency
    						}
    					}
						break;
					#endif
					}
					case id_Iu_Release:
					{
					    if((UEID > 0) && (UEID < UE_MAX_NUM_AUTELAN))
					    {
					    #ifdef IU_SIG_CON_ID
    						/* reset the IuSigConID of UE, book add, 2011-12-16 */
    						iuh_syslog_debug_debug(IUH_DEFAULT, "reset iuSigConId of UE %d\n",UEID);
    						if(sigMsg->CnDomain == CS_Domain)
    							memset(IUH_HNB_UE[UEID]->iuCSSigConId, 0xff, IU_SIG_CONN_ID_LEN);
    						if(sigMsg->CnDomain == PS_Domain)
    							memset(IUH_HNB_UE[UEID]->iuPSSigConId, 0xff, IU_SIG_CONN_ID_LEN);
    					#endif
    					#ifdef RAB_INFO
    					    /* when receive iu release message, release all  */
    						IUH_FREE_RABLIST(IUH_HNB_UE[UEID]->UE_RAB);
    					#endif
    					}
						break;
					}
		            default:{
		                break;
		            }
        	    }
			}
			/* decode message over */

			if(0 == errflag){
                switch(sigMsg->RuaType.ruaType){
                    case Rua_Connect: {
                        iuh_syslog_debug_debug(IUH_DEFAULT,"Rua_Connect\n");
                        if(!IuhAssembleConnect(HNBID, sigMsg, &msgPtr)){
                            iuh_syslog_err("Error: Assemble RUA Connect error\n");
                            errflag = 1;
                        }
                        break;
                    }
                    case Rua_DirectTransfer: {
                        if(!IuhAssembleDirectTransfer(HNBID, sigMsg, &msgPtr)){
                            iuh_syslog_err("Error: Assemble RUA DirectTransfer error\n");
                            errflag = 1;
                        }
                        
                        break;
                    }
                    case Rua_Disconnect: {
                        if(!IuhAssembleDisconnect(HNBID, sigMsg, &msgPtr)){
                            iuh_syslog_err("Error: Assemble RUA Disconnect error\n");
                            errflag = 1;
                        }
                        break;
                    }
                    case Rua_ConnectionlessTransfer: {
						iuh_syslog_debug_debug(IUH_DEFAULT, "@@@ ranap_type = %d\n",sigMsg->ranap_type);
						if(sigMsg->ranap_type == Pro_id_Reset)
						{
							Iuh2IuMsg tmpSigMsg;
							tmpSigMsg.RuaType.ruaType = Pro_ConnectionlessTransfer;
                        	tmpSigMsg.SigType = CONNECTIONLESS_TO_CN;
							tmpSigMsg.RuaType.msgType = RUA_PR_successfulOutcome;
                			tmpSigMsg.RncPlmn.RncId = gRncId;
							tmpSigMsg.CnDomain = sigMsg->CnDomain;
							IuhAssembleRanapResetAck(&tmpSigMsg);
							if(!IuhSendMsg2Iu(&tmpSigMsg,QID)){
			                /* 
        		                          ** if can not send message to iu,
        		                          ** we should give hnb an error indication within RUA message
        		                          */
			                    iuh_syslog_err("Error: @@@ Iuh send message to Iu error\n");
			                }
						}
                        if(!IuhAssembleConnectionlessTransfer(sigMsg, &msgPtr)){
                            iuh_syslog_err("Error: Assemble RUA ConnectionlessTransfer error\n");
                            errflag = 1;
                        }
                        int i;
    					if(0 == errflag){
    	                    for(i = 1; i < HNB_NUM; i++){
    	                        if((IUH_HNB[i] != NULL) && (IUH_OPTIMIZE_PAGING_HNB(i, sigMsg) == 1)){
    	                            if(!IuhSendMessage(i, &msgPtr, RUA_PPID)) {
    	            	                iuh_syslog_err("Error: Send Rua_ConnectionlessTransfer essage\n");
    	            	            }
    	                        }
    	                    }
							errflag = 2;// do nothing
    					}
                        break;
                    }
                    case Rua_ErrorIndicator:{
                        if(!IuhAssembleRuaErrIndication(sigMsg, &msgPtr)){
                            iuh_syslog_err("Assemble RUA ErrorIndication error\n");
                            errflag = 1;
                        }
                        break;
                    }
                    case Rua_privateMessage:{
                        break;
                    }
                    default:
                        break;
                }
            }
            if(0 == errflag){
                iuh_syslog_debug_debug(IUH_DEFAULT,"send message\n");
                if(!IuhSendMessage(HNBID, &msgPtr, RUA_PPID)) {
	                iuh_syslog_err("Error: send RUA message error  HNBID = %d\n",HNBID);
	            }
            }
            else if(1 == errflag){
                sigMsg->cause.present = Cause_protocol;
                sigMsg->cause.choice.protocol = Protocol_abstract_syntax_error_falsely_constructed_message;
                IuhRuaErrorIndication(sigMsg, HNBID);
            }
			
        }
		else if(HNBMsgq.mqinfo.type == CONTROL_DATA){ //control data
			Iuh2IuMsg sigMsg;
			sigMsg.SigType = UE_CONTROL_TO_IU;
			sigMsg.ueop = HNBMsgq.mqinfo.u.ueInfo.op;
			sigMsg.UEID = HNBMsgq.mqinfo.u.ueInfo.ueInfo.UEID;
			memcpy(sigMsg.imsi, HNBMsgq.mqinfo.u.ueInfo.ueInfo.IMSI, IMSI_LEN);
			memcpy(sigMsg.contextid, HNBMsgq.mqinfo.u.ueInfo.ueInfo.ContextID, CONTEXTID_LEN);
			
			if(!IuhSendMsg2Iu(&sigMsg,QID)){ 
                iuh_syslog_err("Error: Iuh send message to Iu failed!!\n");
				if(!IuhSendMsg2Iu(&sigMsg,QID)){ 
	                iuh_syslog_err("Error: Iuh send message to Iu failed again!!!\n"); 
	            }
            }
		}
	}
    iuh_syslog_err(IUH_DEFAULT,"Error: quit loop\n");
}


/*****************************************************
** DISCRIPTION:
**          Assamble RUA error indication message and send msg to HNB
** INPUT:
**          sigMsg
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void IuhRuaErrorIndication(Iuh2IuMsg *causeMsg, int HNBID)
{
	iuh_syslog_err("IuhRuaErrorIndication\n");
    IuhProcotolMsg msgPtr;
    if(!IuhAssembleRuaErrIndication(causeMsg, &msgPtr)){
        iuh_syslog_err("Assemble RUA ErrorIndication error\n");
    }
    if(!IuhSendMessage(HNBID, &msgPtr,RUA_PPID)) {
        iuh_syslog_err("send RUA ErrorIndication to HNB error\n");
    }
	else{
		iuh_syslog_debug_debug(IUH_DEFAULT, "IUH send RUA ErrorIndication to HNB Success!!!\n");
	}
    return;
}


/*****************************************************
** DISCRIPTION:
**          Assamble HNBAP error indication message and send msg to HNB
** INPUT:
**          sigMsg
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void IuhHnbapErrorIndication(HNBAPCause *cause, int HNBID)
{
    IuhProcotolMsg msgPtr;
    if(!IuhAssembleHnbapErrIndication(cause, &msgPtr)){
        iuh_syslog_err("assemble HNBAP ErrorIndication error\n");
    }
    if(!IuhSendMessage(HNBID, &msgPtr, HNBAP_PPID)) {
        iuh_syslog_err("send HNBAP ErrorIndication message error HNBID = %d\n",HNBID);
    }
    return;
}


