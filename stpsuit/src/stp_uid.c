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
* stp_uid.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for socket and packet handling in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/un.h>  
#include <unistd.h>

#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_uid.h"
#include "stp_in.h"
#include "stp_log.h"
#include "stp_mngr.h"

struct sockaddr_un	server , client;
unsigned int uidSock;


int stp_uid_bpduPktSocketInit(void)
{
	memset(&client,0,sizeof(client));
	memset(&server,0,sizeof(server));
	//fromlen = sizeof(server);
	
	if((uidSock = socket(AF_LOCAL,SOCK_DGRAM,0))!= -1)
	{       
		stp_syslog_dbg("RSTP>>#### sock is %d \n", uidSock);
		client.sun_family = AF_LOCAL;
		server.sun_family = AF_LOCAL;

		strcpy(client.sun_path, "/tmp/MSTP_CLIENT");
	    strcpy(server.sun_path, "/tmp/MSTP_SERVER");
		//printf("create socket\n");
		unlink(client.sun_path);

		
		//printf("link addr\n");
		if(bind(uidSock,(struct sockaddr *)&client,sizeof(client))!= -1)
		{	
			chmod(client.sun_path, 0777);
		}
		else
		{
			stp_syslog_dbg("sorry bind error;");
			close (uidSock);
			return 0;
		}
	}
	else 
	{
		stp_syslog_warning("sock init failed\n");
		//close (sock);
	}
     	
	return uidSock;
}

int stp_uid_SocketRecvfrom (void* msg, int buffer_size)
{
	int	byteRecv;
	unsigned int len = sizeof(struct sockaddr_un);

	while(1) 
	{
	    byteRecv = recvfrom(uidSock, msg, buffer_size, 0,(struct sockaddr*) &server ,&len);
	    if(byteRecv < 0 && errno == EINTR) 
	  	{
	  		continue;
	    }
	    break;
	}

	return byteRecv;
}



int stp_uid_SocketSendto(void* msg, int msg_size)
{
  int	rc = 0;
  int size = 0;
  
  stp_syslog_packet_send("into socketSendto,msg_size = %d\n",msg_size);
  while(1) 
  {
     rc = sendto (uidSock, (msg+size), (msg_size-size), 0,(struct sockaddr *) &server ,sizeof(struct sockaddr_un));
     if(rc < 0) 
	 {
        if(errno == EINTR) 
		{
          continue;
        } 
		else 
		{
          return -1; 
        }
     }
	 break;		
  }
  return 0;
}

int stp_uid_bridge_tx_bpdu (int port_index,unsigned char *bpdu, size_t bpdu_len)
{
	int 		i = 0, j = 0;
	UID_MSG_T*	msg;
	unsigned char * bpdu_tmp = NULL;
	unsigned int lLen = 0;

	if(bpdu_len < 64){
		bpdu_len = 64;	
	}

	lLen = sizeof(UID_MSG_T) + bpdu_len;
	
	msg = (UID_MSG_T *)malloc(lLen);
	if(!msg)
		return -1;
	
	memset(msg, 0, lLen);
	msg->port_index  = port_index;
	msg->bodyLen = bpdu_len;
	msg->bpdu = (unsigned char*)(msg +1);
  
 	memcpy (msg->bpdu, bpdu, bpdu_len);
	//printf("all len %d,bpdu len %d, port %d\n",lLen,msg->bodyLen,port_index);
	stp_syslog_packet_send("RSTP>>socket SEND %d bytes packet via port[%#0x]\n",bpdu_len,port_index);
  	stp_syslog_packet_send("#################################################\n");
	for(i=0;i< bpdu_len;i++)
	{
		stp_syslog_packet_send("%02x ",msg->bpdu[i]);
		if((i+1)%16==0)
		{
			stp_syslog_packet_send("\n");
		}
	}
	if(0!=(i%16))
	{
		stp_syslog_packet_send("\n");
	}
    stp_syslog_packet_send("#################################################\n");
	if((stp_uid_SocketSendto (msg, lLen))!=-1)
		stp_syslog_packet_send("stp_uid_SocketSendto succeed!\n");
	else
		stp_syslog_packet_send("stp_uid_SocketSendto failed\n");

	free(msg);
	return 0;
}
	

char stp_uid_read_socket(void)
{
	unsigned char *buff = NULL,*tmp = NULL;
	UID_MSG_T* msg = NULL;
	size_t msgsize;
  	int rc,i;

	buff = (char *)malloc(MAX_UID_MSG_SIZE);
 	if(!buff){
		printf(" stp_uid_read_socket:: Sorry,Malloc error!\n");
  		return -1;
 	}
  	memset(buff, 0, MAX_UID_MSG_SIZE);
		
  	msgsize = stp_uid_SocketRecvfrom (buff, MAX_UID_MSG_SIZE);  	
	if(msgsize <= 0) 
	{
    	stp_syslog_error("Something wrong in UIF ?\n");
    	return 0;
  	}
  
  	msg = (UID_MSG_T*) buff;
	tmp = (unsigned char*)(msg+1);
	stp_syslog_packet_receive("\nRSTP>> socket RCVD %d bytes packet on port[%#0x]\n",msgsize,msg->port_index);
  	stp_syslog_packet_receive("#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.\n");
	for (i=0;i< msg->bodyLen;i++)
	{
	 	stp_syslog_packet_receive("%02x ",tmp[i]);
	 	if ((i+1)%16==0) 
			stp_syslog_packet_receive("\n");
	}
	if(0!=(i%16))
	{
		stp_syslog_packet_receive("\n");
	}
	stp_syslog_packet_receive("#################################################\n");   	
    rc =  stp_in_rx_bpdu (0, msg->port_index, 0, 0,
                    (BPDU_T*) (msg + 1),
                    msg->bodyLen);
	stp_syslog_packet_receive("#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.\n");   	

	free(buff);
	stp_syslog_packet_receive("<<<<<uid:: END recv bpdu port[%d] len[%d]>>>>>\n",msg->port_index,msg->bodyLen);
	return 0;
}
#ifdef __cplusplus
}
#endif

