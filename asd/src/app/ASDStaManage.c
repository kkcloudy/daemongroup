#include "ASDStaManage.h"

int socket_rcv = -1;
int socket_snd = -1;
int maxfd = 1;
unixAddr toMain;

int asd_init_socket_server(char *path)
{
	char ASD_STA_PATH[PATH_LEN] ;
	struct sockaddr_un local;
	int len;
	int fd = -1;

	memset(&ASD_STA_PATH,0,PATH_LEN);
	memcpy(ASD_STA_PATH,path,strlen(path));
	
	InitPath(vrrid,ASD_STA_PATH);
	fd = socket(PF_UNIX,SOCK_DGRAM,0);
	if(fd < 0)
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"func :%s socket init error %s",__func__,strerror(errno));
		exit(1);
	}
	local.sun_family = PF_UNIX;
	strcpy(local.sun_path,ASD_STA_PATH);
	unlink(local.sun_path);
	len = sizeof(local.sun_family) + strlen(local.sun_path);
	if(bind(fd,(struct sockaddr *) &local,len) < 0)
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"func :%s socket bind error %s",__func__,strerror(errno));
		exit(1);
	}
	socket_rcv = fd;
	return fd;
}
int asd_init_send_socket()
{
	char AWSM_PATH[PATH_LEN] = "/var/run/wcpss/asd_table";
	int fd  = -1;
	fd = socket(PF_UNIX,SOCK_DGRAM,0);
	if(fd < 0)
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"func :%s socket init error %s",__func__,strerror(errno));
		exit(1);
	}

	
	InitPath(vrrid,AWSM_PATH);

	toMain.addr.sun_family = PF_UNIX;
	strcpy(toMain.addr.sun_path,AWSM_PATH);
	toMain.addrlen = strlen(toMain.addr.sun_path)+sizeof(toMain.addr.sun_family);

	socket_snd = fd;
	return fd;
}

void *asd_sta_manage()
{	
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd_sta_manage begin!\n");
	asd_pid_write_v2("sta_manage");
	unsigned int count = 0;
	int len = 0;
	fd_set set;
	TableMsg *msg;
	unsigned char buf[2048] = {0};
	char WlanID,RadioID;
	int WTPID,num;
	int i;
	struct asd_data *wasd = NULL;
	struct sta_info *sta = NULL;
	struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM];
	struct manage_stainfo  *sta_manage_list;
	struct manage_stainfo *sta_report_list;
	int bssindex = 0;
	unsigned int radio_g_id = 0;
	sta_manage_list = malloc_init_mac_list();
	if(NULL == sta_manage_list)
		exit(1);
	sta_report_list = malloc_init_mac_list();
	if(NULL == sta_report_list)
		exit(1);
	memset(sta_manage_list,0,sizeof(struct manage_stainfo));
	memset(sta_report_list,0,sizeof(struct manage_stainfo));	
	
	FD_ZERO(&set);
	asd_init_socket_server(ASD_STA_SERVER_PATH);
	asd_init_send_socket();
	if(socket_rcv > maxfd)
		maxfd = socket_rcv;
	FD_SET(socket_rcv,&set);
Loop:
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s Loop..........\n",__func__);
		count = select(maxfd +1,&set,NULL,NULL,(struct timeval *)0 );
		if(count <=0)
			goto Loop;
		if(FD_ISSET(socket_rcv,&set))
		{
			memset(buf,0,sizeof(buf));
			len = recvfrom(socket_rcv,buf,sizeof(buf),0,NULL,0);
			if(len <= 0)
			{
				goto Loop;
			}
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"***************recieve msg**************\n");
			msg = (TableMsg *)buf;
			WlanID = msg->u.bss_sta.wlanid;
			RadioID = msg->u.bss_sta.radioid;
			WTPID = msg->u.bss_sta.wtpid;
			count  = msg->u.bss_sta.count;
			radio_g_id = WTPID*4+RadioID;
			num = ASD_SEARCH_RADIO_STA(radio_g_id,bss);
			pthread_mutex_lock(&(asd_g_sta_mutex)); 		
			for( i= 0 ; i < num ; i++){
				if(bss[i]->WlanID == WlanID)
				{
					wasd = bss[i];
					break;
				}
			}
			if(wasd != NULL)
			{
				bssindex = wasd->BSSIndex;
				sta = wasd->sta_list;
				while(sta)
				{
					if(check_sta_authorized(wasd,sta))
						maclist_add_mac(sta_manage_list,sta->addr);
					sta = sta->next;
				}
			}	
			else
			{
				asd_printf(ASD_LEAVE,MSG_DEBUG,"func:%s: wasd is NULL(wtpid:%d wlanid:%d)!\n",__func__,WTPID,WlanID);
			}
			pthread_mutex_unlock(&(asd_g_sta_mutex));
			
			for(i = 0 ; i < count && i<128; i++)
			{			
				//strncpy(mac,msg->u.bss_sta.mac+i*MAC_LEN,MAC_LEN);
				//memcpy(mac,msg->u.bss_sta.mac+i*MAC_LEN,MAC_LEN);
			maclist_add_mac(sta_report_list,msg->u.bss_sta.mac[i]);
			}
			num = sta_manage_list->num;
			if(sta_manage_list->num <= sta_report_list->num)
				maclist_match(sta_manage_list,sta_report_list);
			else
				maclist_match(sta_report_list,sta_manage_list);
			notice_asd_del_sta (bssindex,num,sta_manage_list);
			notice_ap_del_sta(bssindex,WlanID,WTPID,sta_report_list);
			clean_maclist(sta_manage_list);
			clean_maclist(sta_report_list);
		}
	goto Loop;
	free(sta_manage_list);
	sta_manage_list = NULL;
	free(sta_report_list);
	sta_report_list  = NULL;
	return NULL;
}
struct manage_stainfo *malloc_init_mac_list()
{	
	struct manage_stainfo  *maclist = NULL;
	int i;
	maclist = malloc(sizeof(struct manage_stainfo));
	if(NULL == maclist)
		exit(1);
	memset(maclist,0,sizeof(struct manage_stainfo));
	maclist->mac_list = NULL;
	maclist->num = 0;
	for(i = 0; i < STA_HASH_SIZE ;i++)
		maclist->sta_hash[i] = NULL;
	return maclist;
}
int clean_maclist(struct manage_stainfo *maclist)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s\n",__func__);

	if(!maclist ||maclist->num == 0)
		return -1;
	struct mac_info *macinfo = NULL;
	macinfo = maclist->mac_list;
	while(macinfo)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"macinfo mac : "MACSTR"\n",MAC2STR(macinfo->mac));
		maclist_del_mac(maclist,macinfo->mac);
		macinfo = macinfo->next;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"macinfo num now :%d\n",maclist->num);
	return 0;
}
int notice_asd_del_sta(int bssindex,int num,struct manage_stainfo *maclist)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s\n",__func__);
	if(!maclist)
		return -1;
	struct mac_info *macinfo = NULL;
	int i = 0;
	int ret;
	macinfo = maclist->mac_list;
	if(!macinfo)
		return -1;
	TableMsg msg;
	again:
		memset(&msg,0,sizeof(msg));
		msg.Type = STA_TYPE;
		msg.Op = STA_CHECK_DEL;
		msg.u.STAINFO[0].BSSIndex = bssindex;
		msg.u.STAINFO[0].count = maclist->num;
		msg.u.STAINFO[0].sta_num = num;
		for( i = 0 ; i < 64&&i<maclist->num&&macinfo;i++)
		{
			memcpy(msg.u.STAINFO[i].STAMAC,macinfo->mac,MAC_LEN);
			macinfo = macinfo->next;
		}
		ret = sendto(socket_snd,&msg,sizeof(msg)+1,0,(struct sockaddr *)&toMain.addr,toMain.addrlen);
		if(ret < 0)
		{
			asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s : sendto main thread del sta error:%s\n",__func__,strerror(errno));
		}
		else
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s : sendto asd main thread sucess!\n",__func__);
		if((64 == i)&&(macinfo != NULL))
			goto again;
	return 0;
}
int notice_ap_del_sta(int bssindex,unsigned char wlanid ,int wtpid ,struct manage_stainfo *maclist)
{
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s\n",__func__);
	TableMsg msg; 
	int ret,len ;
	struct mac_info *macinfo;
	macinfo = maclist->mac_list;
	if(macinfo == NULL)
		return -1;
	memset(&msg , 0,sizeof(msg));

	msg.Type = STA_TYPE;
	msg.Op = WID_DEL;
	msg.u.STA.BSSIndex = bssindex;
	msg.u.STA.WTPID  = wtpid;
	msg.u.STA.wlanId = wlanid;
	len = sizeof(msg);
	while(macinfo)
	{
		memset(msg.u.STA.STAMAC,0,MAC_LEN);
		memcpy(msg.u.STA.STAMAC,macinfo->mac,MAC_LEN);
		ret = sendto(socket_snd,&msg,len,0,(struct sockaddr *)&toWID.addr,toWID.addrlen);
		if(ret < 0)
		{
			asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s : sendto wid thread del sta error:%s\n",__func__,strerror(errno));		
		}
		else
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s : sendto wid thread del sta"MACSTR" sucess!\n",__func__,MAC2STR(macinfo->mac));
		macinfo  = macinfo->next;
	//	sleep(2);
	}
	return 0;
}
int maclist_match(struct manage_stainfo *sta_manage_list,struct manage_stainfo *sta_report_list)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s\n",__func__);
	struct mac_info *sta_r,*sta_m,*tmp;
	tmp = NULL;
	sta_r = NULL;
	sta_m = NULL;
	sta_r = sta_manage_list->mac_list;
	while(sta_r)
	{
		sta_m = maclist_get_mac(sta_report_list,sta_r->mac);
		if(sta_m != NULL)
		{
			tmp = sta_r->next;
			maclist_del_mac(sta_manage_list,sta_r->mac);
			sta_r = NULL;
			maclist_del_mac(sta_report_list,sta_m->mac);
			sta_m = NULL;
		}
		sta_r = tmp;
	}
	return 0;
}
int maclist_add_mac(struct manage_stainfo *maclist,const unsigned char *mac)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s\n",__func__);
	struct mac_info *sta = NULL;
	sta = maclist_get_mac(maclist,mac);
	if(sta != NULL)
		return -1;
	sta = malloc(sizeof(struct mac_info));
	memset(sta,0,sizeof(struct mac_info));
	memcpy(sta->mac,mac,MAC_LEN);
	sta->next = NULL;
	sta->hnext = NULL;
	
	sta->next  = maclist->mac_list;
	maclist->mac_list = sta;
	mac_hash_add_mac(maclist,sta);

	maclist->num++;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s :  sta "MACSTR"\n",__func__,MAC2STR(mac));
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"num = %d\n",maclist->num);
	return 0;
}
struct mac_info * maclist_get_mac(struct manage_stainfo *maclist,const unsigned char *mac)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s :  sta "MACSTR"\n",__func__,MAC2STR(mac));
	struct mac_info *sta = NULL;
	sta = maclist->sta_hash[STA_HASH(mac)];
	while(sta)
	{
		if(0 == memcmp(sta->mac,mac,MAC_LEN) )
			return sta;
		sta = sta->hnext;
	}
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s : "MACSTR" has not get!\n",__func__,MAC2STR(mac));
	return NULL;
}
int maclist_del_mac(struct manage_stainfo *maclist,const unsigned char *mac)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s :  sta "MACSTR"\n",__func__,MAC2STR(mac));
	struct mac_info *sta,*tmp;
	sta  = maclist_get_mac(maclist,mac);
	if(sta == NULL)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta "MACSTR" is not get!\n",MAC2STR(mac));
		return -1;
	}
	if(0 == memcmp(maclist->mac_list->mac,sta->mac,MAC_LEN))
		maclist->mac_list = sta->next;
	tmp = maclist->mac_list;
	mac_hash_del_mac(maclist,mac);
	
	while(tmp&&tmp->next)
	{
		if(tmp->next == sta)
		{
			tmp->next = sta->next;
			sta->next = NULL;
		}
		tmp = tmp->next;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s : delete  sta "MACSTR"\n",__func__,MAC2STR(mac));
	free(sta);
	sta = NULL;
	maclist->num--;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s : num = %d\n",__func__,maclist->num);
	return 0;	
}
int mac_hash_add_mac(struct manage_stainfo *maclist,struct mac_info *sta)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s :  sta "MACSTR"\n",__func__,MAC2STR(sta->mac));
	sta->hnext = maclist->sta_hash[STA_HASH(sta->mac)];
	maclist->sta_hash[STA_HASH(sta->mac)] = sta;
	return 0;
}
int mac_hash_del_mac(struct manage_stainfo *maclist,const unsigned char *mac)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s :  sta "MACSTR"\n",__func__,MAC2STR(mac));
	struct mac_info *s;
	s = maclist->sta_hash[STA_HASH(mac)];
	if(!s) return -1;
	if(0 == memcmp(s->mac,mac,MAC_LEN) )
	{
		maclist->sta_hash[STA_HASH(mac)]  = s->hnext;
		return 0;
	}
	while((s->hnext != NULL)&&(memcmp(s->hnext->mac,mac,MAC_LEN) != 0))
		s = s->hnext;
	if(NULL == s->hnext) return -1;
	s->hnext = s->hnext->hnext;
	return 0;
}
