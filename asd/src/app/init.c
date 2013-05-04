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
* AsdInit.c
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

/* Standard C library includes */
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

/* Network Includes */
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <asm/types.h>
#include <linux/netlink.h>


#ifdef MEM_DEBUG
	#include "memwatch.h"
#else
	#include <stdlib.h>
#endif
#include "common.h"
#include "asd.h"
#include "ASDStaInfo.h"

#include "include/pack.h"
//#include "include/typedef.h"
#include "include/raw_socket.h"
#include "include/proc.h"
#include "include/debug.h"
#include "include/alg_comm.h"
#include "include/auth.h"
#include "include/wai_sta.h"
#include "include/certupdate.h"
#include "include/cert_auth.h"
#include "include/cgi_server.h"
#include "include/init.h"
#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"


/*
 * Extract the interface name out of /proc/net/wireless or /proc/net/dev.
 */
static inline char *
iw_get_ifname(char *	name,	/* Where to store the name */
				int	nsize,	/* Size of name buffer */
				char *	buf)	/* Current position in buffer */
{
	char *	end;

	/* Skip leading spaces */
	while(isspace(*buf))
	buf++;

	end = strstr(buf, ":");

	/* Not found ??? To big ??? */
	if((end == NULL)) 
	{
		printf("iw_get_ifname failed! end == NULL\n");
		return(NULL);
	}
	if(((end - buf) + 1) > nsize)
	{
		printf("iw_get_ifname failed!(end - buf) + 1=%d,nsize=%d\n",(end - buf) + 1,nsize);
		return(NULL);
	}
	/* Copy */
	memcpy(name, buf, (end - buf));
	name[end - buf] = '\0';
	return(end + 2);
}


int get_wlandev(char *name, int len)
{
	char	buff[1024];
	FILE *fh;
	int ret = 0;
	fh = fopen(PROC_SYS_DEV_WIFI, "r");

	if(fh != NULL)
	{
		char *s = NULL;
		/* Success : use data from /proc/net/wireless */
		/* Eat 2 lines of header */
		fgets(buff, sizeof(buff), fh);
		fgets(buff, sizeof(buff), fh);
		/* Read each device line */
    		while(fgets(buff, sizeof(buff), fh))
    		{
			/* Extract interface name */
			s = iw_get_ifname(name, len, buff);
			if(!s)
			{
				fprintf(stderr, "Cannot parse " PROC_SYS_DEV_WIFI "\n");
				break;
			}
			if(memcmp(name,"wifi1",5) ==0)
			{
				ret = 1;
				break;
			}
    		}
		fclose(fh);
	}
	return ret;
 }

/*初始化AP 的WAPI信息元素*/
int ap_initialize_wie(apdata_info *papdata)
{/*初始化WIE 参数*/
#define ADDSHORT(frm, v) do {frm[0] = (v) & 0xff;frm[1] = (v) >> 8;} while (0)
#define ADDSELECTOR(frm, sel) do {memcpy(frm, sel, 4); frm += 4;} while (0)

	u8	 *p = papdata->wie_ae, flag1 = 0, flag2 = 0, *q;
	u8 akm_haha1[4] = {0x00, 0x14, 0x72, 0x01};/*WAI证书鉴别和密钥管理*/
	u8 akm_haha2[4] = {0x00, 0x14, 0x72, 0x02};/*WAI共享密钥鉴别和密钥管理*/
	
	memset(p, 0, 255);
	*p = 68;
        p += 1;
	p += 1;/*length*/
	ADDSHORT(p,1);
	p += sizeof(u16);
	p += sizeof(u16);
	if(papdata->wai_policy & 0x08 /*bit 3:WAI*/)
	{
		flag1 = 1;
		ADDSELECTOR(p, akm_haha1);
	}
	if(papdata->wai_policy & 0x04 /*bit 2:PSK*/)
	{
		flag2 = 1;
		ADDSELECTOR(p, akm_haha2);
	}
	q = papdata->wie_ae + 4;
	ADDSHORT(q ,flag1 + flag2);

	flag1 = 0;
	flag2 = 0;
	
	p += sizeof(u16);
	if(papdata->wai_policy & 0x02 /*bit 1:单播密码套件:WPI-SMS4*/)
	{
		flag1 = 1;
		ADDSELECTOR(p, akm_haha1);
	}
	q = p -sizeof(u16) - flag1 * 4;
	ADDSHORT(q, flag1);//*(u16 *)(p -sizeof(u16) - flag1 * 4)= flag1;/*set key no*/
	
	if(papdata->wai_policy & 0x01 /*bit 0:组播密码套件:WPI-SMS4*/)
	{
		ADDSELECTOR(p, akm_haha1);
	}
	/*set capability*/
	if(papdata->wai_policy & 0x10 /*bit 4:pre-auth*/)
	{
		ADDSHORT(p , 1);
	}
	p += sizeof(u16);
	/*set wie length*/
	papdata->wie_ae[1] = p - papdata->wie_ae - 2;
	return 0;
}
/*初始化AP的签名算法参数*/
int ap_initialize_alg(apdata_info *papdata)
{/*初始化alg 参数*/
	char alg_para_oid_der[16] = {0x06, 0x09,0x2a,0x81,0x1c, 0xd7,0x63,0x01,0x01,0x02,0x01};
	
	memset((u8 *)&(papdata->sign_alg), 0, sizeof(wai_fixdata_alg));
	papdata->sign_alg.alg_length = 16;
	papdata->sign_alg.sha256_flag = 1;
	papdata->sign_alg.sign_alg = 1;
	papdata->sign_alg.sign_para.para_flag = 1;
	papdata->sign_alg.sign_para.para_len = 11;
	memcpy(papdata->sign_alg.sign_para.para_data, alg_para_oid_der, 11);
	return 0;
}
void free_one_interface(struct wapid_interfaces *interfaces )
{
		if(interfaces->identity)
		{
			free(interfaces->identity);
			interfaces->identity = NULL;
		}
		if(interfaces->ssid)
		{
			free(interfaces->ssid);
			interfaces->ssid = NULL;
		}
		if(interfaces->password)
		{
			free(interfaces->password);
			interfaces->password= NULL;
		}
		if(interfaces->wapid)//qiuchen
		{
			if(interfaces->wapid->wapi_sta_info)
			{
				free(interfaces->wapid->wapi_sta_info);
				interfaces->wapid->wapi_sta_info= NULL;
			}	
			free(interfaces->wapid);
			interfaces->wapid= NULL;
		}
}


void free_one_wapi(struct asd_wapi * wapi_wasd)
{
	if(wapi_wasd->cert_info.ap_cert_obj !=NULL){
		if(wapi_wasd->cert_info.ap_cert_obj->cert_bin !=NULL){
			free(wapi_wasd->cert_info.ap_cert_obj->cert_bin);
			wapi_wasd->cert_info.ap_cert_obj->cert_bin = NULL;
		}
		free(wapi_wasd->cert_info.ap_cert_obj);
		wapi_wasd->cert_info.ap_cert_obj = NULL;
	}
	free(wapi_wasd);
	wapi_wasd = NULL;
}

void free_all(struct asd_wapi *tmp_circle)
{
	/*释放所有资源*/
	struct wapid_interfaces *interfaces = NULL,*tmp_interfaces= NULL;
	if(tmp_circle == NULL) return;
	interfaces = tmp_circle->vap_user;
	while(interfaces)
	{
		DPrintf("free_all\n");
		free_one_interface(interfaces );
		tmp_interfaces = interfaces;
		interfaces = interfaces ->next;
		memset(tmp_interfaces, 0, sizeof(*tmp_interfaces));
		free(tmp_interfaces);
		tmp_interfaces = NULL;
		tmp_circle->vap_user = interfaces;
	}
	/*释放所有资源*/	
	//interfaces = tmp_circle->vap_11a_user;
	while(interfaces)
	{
		DPrintf("free_all 11a\n");
		free_one_interface(interfaces );
		tmp_interfaces = interfaces;
		interfaces = interfaces ->next;
		memset(tmp_interfaces, 0, sizeof(*tmp_interfaces));
		free(tmp_interfaces);
		tmp_interfaces = NULL;
		//tmp_circle->vap_11a_user = interfaces;
	}
}

/*AP初始化*/
int ap_initialize(apdata_info *pap)
{
	int ret = 0;
	//int i = 0;
	
	struct wapid_interfaces *tmp_wapid;
	struct asd_wapi*tmp_circle;
	
	assert(pap!=NULL);
	tmp_wapid = (struct wapid_interfaces *)pap->user_data;
	tmp_circle = (struct asd_wapi *)tmp_wapid->circle_save;

	/*初始化msksa*/
	get_random(pap->msksa.msk, MULTI_KEY_LEN);
	pap->msksa.mskid= 0;

	/*初始化gnonce*/
	pap->gnonce[0] = 0x5c365c36;
	pap->gnonce[1] = 0x5c365c36;
	pap->gnonce[2] = 0x5c365c36;
	pap->gnonce[3] = 0x5c365c36;
	//DPrintf("wai_policy = %d\n", pap->wai_policy);
	pap->ap_debug = debug;
	if( pap->wai_policy&0x04)
		ap_pskbk_derivation(pap);
	/*初始化AP 的WAPI信息元素*/
	ap_initialize_wie(pap);
	/*初始化AP的签名算法参数*/
	ap_initialize_alg(pap);
	pap->group_No = 0xFFFF;
	memset(pap->gsn, 0, 16);
	return ret;
#if 0
err:
	printf("ap_initialize failed\n");
	return -1;
#endif
}

/*初始化STA信息记录表*/
void ap_initialize_sta_table(apdata_info *pap)
{
	int i = 0;
	struct auth_sta_info_t *psta_table = pap->wapi_sta_info;
	assert(psta_table != NULL);
/*
	DPrintf("sta table has max %d users and each size is %d\n",
		MAX_AUTH_MT_SIMU,sizeof(struct auth_sta_info_t));*/

	for(i = 0; i < MAX_AUTH_MT_SIMU; i++)
	{
		/*重新初始化STA信息*/
		reset_sta_info(&psta_table[i], pap);
	}
}

/*初始化鉴别与密钥管理套间*/
int ap_initialize_akm(apdata_info *apdata)
{
#define BIT(x) (1 << (x))
	struct wapid_interfaces *wapid ;
	struct asd_wapi*tmp_circle ;

	wapid = (struct wapid_interfaces *)apdata->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	/*判断AP是否启用鉴别与密钥管理套间*/
	if(!(apdata->wai_policy &(BIT(2) |BIT(3))))
	{
		return 0;
	}
	/*设置MEK, MAK*/
	set_mcastkey(apdata, &apdata->msksa);
	return 0;
#undef BIT
}

									 
/*创建与ASU通信的套接字*/
 int open_socket_for_asu() 
{
	int sock;
	struct protoent *protocol;
	
	protocol = getprotobyname("udp");
	
	if (protocol == NULL) 
	{
		printf("\nCreat AS_udp socket error 1!!!\n\n");
		exit(-1);
	}
	
	sock = socket(PF_INET, SOCK_DGRAM, protocol->p_proto);
	
	if (sock == -1 ) 
	{
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	}
	return sock;
}

/*创建向Driver发送信息的套接字*/
int socket_open_for_ioctl()
{
	
	int ipx_sock = -1;				/* IPX socket					*/
	int ax25_sock = -1; 			/* AX.25 socket 				*/
	int inet_sock = -1; 			/* INET socket					*/
	int ddp_sock = -1;				/* Appletalk DDP socket 		*/
	
	/*
	* Now pick any (exisiting) useful socket family for generic queries
	* Note : don't open all the socket, only returns when one matches,
	* all protocols might not be valid.
	* Note : in 99% of the case, we will just open the inet_sock.
	* The remaining 1% case are not fully correct...
	*/
	inet_sock=socket(AF_INET, SOCK_DGRAM, 0);
	if(inet_sock!=-1)
	{
		return inet_sock;
	}
	ipx_sock=socket(AF_IPX, SOCK_DGRAM, 0);
	if(ipx_sock!=-1)
	{
		return ipx_sock;
	}
	ax25_sock=socket(AF_AX25, SOCK_DGRAM, 0);
	if(ax25_sock!=-1)
	{
		return ax25_sock;
	}
	
	ddp_sock=socket(AF_APPLETALK, SOCK_DGRAM, 0);
	return ddp_sock;
}

/*创建接收Driver信息的套接字*/
int socket_open_for_netlink()
{
	int sock = -1; 
	int bindsock = -1;
	struct sockaddr_nl nlskaddr;
	
	sock = socket(AF_NETLINK,SOCK_RAW,NETLINK_USERSOCK);
	
	if(sock > 0)
	{
		memset ( &nlskaddr, 0 , sizeof( nlskaddr ));
		nlskaddr.nl_family = (sa_family_t)AF_NETLINK;
		nlskaddr.nl_pid = (__u32)getpid();
		nlskaddr.nl_groups = COMMTYPE_GROUP;
	}
	else{
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	} 
	
	bindsock = bind(sock,(struct sockaddr *)&nlskaddr,sizeof(nlskaddr));
	
	if(bindsock!=0)
	{
		DPrintf("bind failure!\n");
		close(sock);//qiuchen
		return -1;
    }
	return sock;
}

/*设置鉴别与密钥管理套间*/
int set_wapi(apdata_info *pap)
{
	struct ioctl_drv ioctl_drv_data;
	int res = 0;
	
	memset(&ioctl_drv_data, 0, sizeof(struct ioctl_drv));
	ioctl_drv_data.io_packet = P80211_PACKET_WAPIFLAG;
	ioctl_drv_data.iodata.wDataLen = 1;
	ioctl_drv_data.iodata.pbData[0] = pap->wai_policy;
	res=wapid_ioctl(pap, P80211_IOCTL_SETWAPI_INFO, &ioctl_drv_data, sizeof(struct ioctl_drv));
	return res;
}

