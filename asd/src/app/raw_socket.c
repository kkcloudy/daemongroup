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
* AsdRawSocket.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*network include*/
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/if.h>
#include <linux/errqueue.h>

#include "asd.h"
#include "ASDStaInfo.h"
#include "ASDCallback_asd.h"

#include "include/raw_socket.h"
#include "include/debug.h"
#include "include/auth.h"
#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"



/*获得设备的索引号*/
int get_device_index_by_raw_socket(char* dev_name, int sock)
{

	char * device_name = dev_name;
	struct ifreq ifr;
	int g_ifr;
	/* Get device index */
	if (strlen (device_name)>= sizeof (ifr.ifr_name)) {
		return RAW_SOCK_ERR_INVALID_DEV_NAME;
	}
	memcpy (ifr.ifr_name, device_name, strlen (device_name) +1);
	if (ioctl (sock, SIOCGIFINDEX, &ifr) != 0) {
		return RAW_SOCK_ERR_GET_IFR;
	}
	g_ifr = ifr.ifr_ifindex;
	return g_ifr;
}
/*获得设备的MAC地址*/
int get_device_mac_by_raw_socket(char* dev_name, int sock, u8 *mac_out)
{	
	
	char * device_name = dev_name;
	struct ifreq ifr;

	if(mac_out == NULL){
		DPrintf("mac_out==NULL\n");
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));

	if (strlen (device_name)>= sizeof (ifr.ifr_name)) {
		return RAW_SOCK_ERR_INVALID_DEV_NAME;
	}
	memcpy (ifr.ifr_name, device_name, strlen (device_name)+1 );

	/* get the dev_name MACADDR */
	if (ioctl (sock, SIOCGIFHWADDR, &ifr) != 0) {
		DPrintf("RAW_SOCK_ERR_GET_MAC\n");
		return RAW_SOCK_ERR_GET_MAC;
	}
	memcpy(mac_out, ifr.ifr_hwaddr.sa_data, 6);
	return 0;
}
/*获得设备的MTU值*/
int get_device_mtu_by_raw_socket(char* dev_name, int sock, u16 *mtu_out)
{	
	
	char * device_name = dev_name;
	struct ifreq ifr;
	
	memset(&ifr, 0, sizeof(struct ifreq));

	if (strlen (device_name)>= sizeof (ifr.ifr_name)) {
		return RAW_SOCK_ERR_INVALID_DEV_NAME;
	}
	memcpy (ifr.ifr_name, device_name, strlen (device_name)+1);

	/* get the dev_name MACADDR */
	if (ioctl (sock, SIOCGIFMTU, &ifr) != 0) {
		return RAW_SOCK_ERR_GET_MAC;
	}
	*mtu_out = ifr.ifr_mtu;
	return 0;
}

/*发送以太网原始数据包*/
int send_rs_data(const void *data, int len, struct ethhdr *eh, apdata_info *pap)
{
	int i = 0;
	int  slen = 0;
	int frame_no = 0;
	int frame_len = 0;
	int mtu = 0;//qiuchen
	u8 packet_type;
	u8 *frame_data = NULL;
	struct sockaddr_ll eth_addr;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	struct asd_data *wasd;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);
	if(pap == NULL) return -1;
	mtu = pap->mtu;//qiuchen
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	wasd = tmp_circle->wasd_bk;
	packet_type = ((char *)data)[3];
	/*构造以太网头*/
	memset(&eth_addr, 0, sizeof(struct sockaddr_ll));
	//eth_addr.sll_family = AF_PACKET;
	//eth_addr.sll_protocol = tmp_circle->g_eth_proto;
	//eth_addr.sll_ifindex =pap->g_ifr;
	eth_addr.sll_halen = ETH_ALEN;
//	eh->h_proto = tmp_circle->g_eth_proto;
	memcpy (eh->h_source, pap->macaddr,  ETH_ALEN);
	if(len + ETH_HLEN <= mtu)/*不需要分片*/
	{
		printf("we need not fragment\n");
		asd_send_wapi(wasd,eh->h_dest,data,len,0,wasd->own_addr);
		return len;
	}else{
		/*分片处理*/
		
		printf("we need fragment\n");
		if(((len - sizeof(packet_head))%(mtu - sizeof(packet_head) -ETH_HLEN ))!=0)
			frame_no = (len - sizeof(packet_head))/(mtu - sizeof(packet_head) -ETH_HLEN ) + 1;
		else			
			frame_no = (len - sizeof(packet_head))/(mtu - sizeof(packet_head) -ETH_HLEN );
		
		frame_data = malloc(frame_no * mtu);
		memset(frame_data , 0x11, frame_no * mtu);//qiuchen
		for(i = 0; i < frame_no; i++)
		{	
			int send_frame_len =0;
			u8 * ethernet_header = frame_data + i * mtu;
			packet_head *wai_frame_header = (packet_head *)( ethernet_header + ETH_HLEN );
			DPrintf("------------flag----mtu=%d---frame_no=%d-----\n",mtu,frame_no);
			/*set ethernet header*/
			memcpy(ethernet_header, eh,  ETH_HLEN);
			/*Copy WAI header*/
			memcpy(wai_frame_header , data, sizeof(packet_head));
			/*set frame sequence*/
			wai_frame_header->frame_sc = i;
			if(i != frame_no -1) /*not the last frame*/
			{	
				/*set frame flag*/
				wai_frame_header->flag |= 0x01;
				/*Copy WAI data*/
				memcpy((u8 *)wai_frame_header + sizeof(packet_head), 
					data + sizeof(packet_head)+ i* (mtu - sizeof(packet_head) -ETH_HLEN ),
					mtu - sizeof(packet_head) -ETH_HLEN );
				/*set WAI len*/
				wai_frame_header->data_len =htons(mtu -ETH_HLEN) ;
				/*set send data len*/
				frame_len = mtu; 
				printf("start fragment\n");
				asd_send_wapi(wasd,eh->h_dest,(u8 *)wai_frame_header,mtu-ETH_HLEN,0,wasd->own_addr);
				slen += mtu-ETH_HLEN;
			}else{
				/*the last frame*/
				u16 temp_len = 0;
				memcpy((u8 *)wai_frame_header + sizeof(packet_head), 
					data + sizeof(packet_head)+ i* (mtu - sizeof(packet_head) -ETH_HLEN ),
					len - sizeof(packet_head) -i* (mtu - sizeof(packet_head) -ETH_HLEN )) ;
				/*set WAI len*/
				temp_len = len  -i* (mtu - sizeof(packet_head) -ETH_HLEN );
				wai_frame_header->data_len =htons(temp_len) ;
				/*set send data len*/
				frame_len =temp_len + ETH_HLEN ; 				
				printf("end fragment\n");
				asd_send_wapi(wasd,eh->h_dest,(u8 *)wai_frame_header,temp_len,0,wasd->own_addr);
				}
			send_frame_len = frame_len - ETH_HLEN;
			slen = slen + send_frame_len ;/*many headers length*/
		}
		slen -=(frame_no - 1)*sizeof(packet_head) ;
	}
	
	free(frame_data);
	return slen;
}
/*接收数据时的错误处理*/
int handle_recverr(int sock)
{
	//BYTE recverr_buf[BUF_LEN] = {0,};
	
	char *msg = NULL;
	struct msghdr mdr;
	struct iovec iov;
	union{
		struct cmsghdr cmsg;
		char control[CMSG_SPACE(sizeof(struct sock_extended_err))];
	}control_un;
	
	struct cmsghdr *cmsg = NULL;
	int  errdata_len = 0;
	
	//struct sock_extended_err  err_data;
	/*
	#define SO_EE_ORIGIN_NONE	      0
      	#define SO_EE_ORIGIN_LOCAL      1
      	#define SO_EE_ORIGIN_ICMP	      2
      	#define SO_EE_ORIGIN_ICMP6      3
	*/
	/*
	struct sock_extended_err
	{
		u_int32_t	  ee_errno;   // error number /
		u_int8_t	  ee_origin;   // where the error originated 
		u_int8_t	  ee_type;    // type 
		u_int8_t	  ee_code;   // code 
		u_int8_t	  ee_pad;   
		u_int32_t	  ee_info;    // additional information 
		u_int32_t	  ee_data;   // other data 
	// More data may follow /
	};
	*/
	
	msg = (char *)malloc(MAXMSG);
	if(msg == NULL)
	{
		printf("malloc failuer\n");
		return -1;
	}
	memset(msg, 0, MAXMSG);
	memset(&mdr,0,sizeof(mdr));//qiuchen
	iov.iov_base = msg;
	iov.iov_len = MAXMSG;
	mdr.msg_name = NULL;
	mdr.msg_namelen = 0;
	mdr.msg_iov = &iov;
	mdr.msg_iovlen = 1;
	mdr.msg_control = &control_un.control;
	mdr.msg_controllen = sizeof(control_un.control);
	
	errdata_len = recvmsg(sock, &mdr, MSG_ERRQUEUE);	

	if(errdata_len < 0) 
	{
		memset(msg, 0, MAXMSG);
		free(msg);
		return -1;
	}
	
	DPrintf("Received %d bytes from\n ", errdata_len); 
	for(	cmsg = CMSG_FIRSTHDR(&mdr); cmsg != NULL; 
		cmsg = CMSG_NXTHDR(&mdr, cmsg))
	{
		if(	(cmsg->cmsg_level == SOL_IP) && 
			(cmsg->cmsg_type == IP_RECVERR))
		{
			struct sock_extended_err  *perr_data = NULL;
			perr_data = (struct sock_extended_err *)CMSG_DATA(cmsg);
			DPrint_string_array("DUMP sock_extended_err ", perr_data, sizeof(struct sock_extended_err));
			DPrintf("sock_extended_err is %08x\n", perr_data->ee_errno);
			DPrintf("sock_extended_err is %02x\n", perr_data->ee_type);
			DPrintf("sock_extended_err is %02x\n", perr_data->ee_code);
			DPrintf("sock_extended_err is %02x\n", perr_data->ee_origin);
			DPrintf("sock_extended_err is %08x\n", perr_data->ee_info);
			DPrintf("sock_extended_err is %08x\n", perr_data->ee_data);
		}
	}
	DPrintf("msg is\n");print_string(msg, errdata_len);
	memset(msg, 0, MAXMSG);
	free(msg);
	return 0;
	
}
/*接收以太网原始数据包*/
/*struct ethhdr
　　{
　　unsigned char h_dest[ETH_ALEN]；
　　unsigned char h_source[ETH_ALEN]；
　　unsigned short h_proto；
}*/

int recv_rs_data(void * data, int buflen, struct ethhdr *_eh, int sk)
{
	int recv_len = 0;

	/*接收数据*/
	recv_len = recvfrom (sk, data, buflen, 0, NULL, NULL);

	/*检查包头*/
	if(recv_len >= ETH_HLEN+12)
	{
		/*copy以太网头*/
		memcpy(_eh, data, ETH_HLEN);
		_eh->h_proto = ntohs (_eh->h_proto);
		recv_len -= ETH_HLEN;
		memmove(data, (char *)data+ETH_HLEN, recv_len);
		return recv_len;

	}
	else if (recv_len <= 0) 
	{
		printf("RAW_SOCK_ERR_RECV\n");
		return RAW_SOCK_ERR_RECV;
	}
	else	if (recv_len <= ETH_HLEN) 
	{
		printf("ETHER frame  too short(%d)\n", recv_len);
		return RAW_SOCK_ERR_RECV;
	}
	else	if (recv_len <= ETH_HLEN+12/*WAI head*/) 
	{
		printf("WAI msg  too short (%d)\n", recv_len);
		return RAW_SOCK_ERR_RECV;
	}
	return RAW_SOCK_ERR_RECV;
}

