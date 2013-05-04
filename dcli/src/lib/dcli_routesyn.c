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
* dcli_routesyn.c
*
* MODIFY:
*		by <qinhs@autelan.com> on 03/07/2008 revision <0.1>
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for routesyn module.
*
* DATE:
*		03/18/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.30 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>

#include "command.h"
#include "if.h"

#include "dcli_routesyn.h"

extern DBusConnection *dcli_dbus_connection;


/* Convert IP address's netmask such as 255.255.255.0 ,into integer. */

unsigned char get_ip_masklen (const char *cp)
{
  unsigned int netmask;
  unsigned char len;
  unsigned char *pnt;
  unsigned char *end;
  unsigned char val;

  if(1 != inet_atoi(cp,&netmask))
  {
		return 0;
  }

  len = 0;
  pnt = (unsigned char *) &netmask;
  end = pnt + 4;

  while ((pnt < end) && (*pnt == 0xff))
    {
      len+= 8;
      pnt++;
    } 

  if (pnt < end)
    {
      val = *pnt;
      while (val)
	{
	  len++;
	  val <<= 1;
	}
    }
  return len;
}



int
inet_atoi (const char *cp, unsigned int  *inaddr)
{
  int dots = 0;
  register u_long addr = 0;
  register u_long val = 0, base = 10;

  do
    {
      register char c = *cp;

      switch (c)
	{
	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
	  val = (val * base) + (c - '0');
	  break;
	case '.':
	  if (++dots > 3)
	    return 0;
	case '\0':
	  if (val > 255)
	    return 0;
	  addr = addr << 8 | val;
	  val = 0;
	  break;
	default:
	  return 0;
	}
    } while (*cp++) ;

  if (dots < 3)
    addr <<= 8 * (3 - dots);
  if (inaddr)
    *inaddr = htonl (addr);
  return 1;
}

static void dcli_route_show_drvroute
(
	struct vty *vty,
	unsigned int dip,
	unsigned int maskLen
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int nexthopindex = 0,ifindex = 0;
	unsigned char isTrunk = 0;
	unsigned short vid = 0;
	unsigned char	macAddr[6] = {0},devNum = 0,portNum = 0;
	int op_ret = 0;
	unsigned int refCnt = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,RTDRV_DBUS_OBJPATH,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_ENTRY);
		
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					 DBUS_TYPE_UINT32,&dip,
					 DBUS_TYPE_UINT32,&maskLen,
					 DBUS_TYPE_INVALID);
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	if (!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_UINT32, &nexthopindex,
							DBUS_TYPE_UINT16, &vid,
							DBUS_TYPE_UINT32, &ifindex,
							DBUS_TYPE_BYTE, &isTrunk,							
							DBUS_TYPE_BYTE, &devNum,
							DBUS_TYPE_BYTE, &portNum,
							DBUS_TYPE_BYTE, &macAddr[0],
							DBUS_TYPE_BYTE, &macAddr[1],
							DBUS_TYPE_BYTE, &macAddr[2],
							DBUS_TYPE_BYTE, &macAddr[3],
							DBUS_TYPE_BYTE, &macAddr[4],
							DBUS_TYPE_BYTE, &macAddr[5],
							DBUS_TYPE_UINT32,&refCnt,
							DBUS_TYPE_INVALID)) 
	{
			vty_out(vty,"can't get the route entry msg.\n");
	}	
	dbus_message_unref(reply);
	dip = ntohl(dip);
	if((0 == op_ret) || 
		(DCLI_ROUTE_RETCODE_ACTION_TRAP2CPU == op_ret)||
		(DCLI_ROUTE_RETCODE_ACTION_HARD_DROP== op_ret)) {
		vty_out(vty,"%-16s%-5s%-7s%-8s%-7s%-7s%-6s%-18s%-5s\n",		\
				"===============","====","======","=======","======","======","=====","=================","=====");
		vty_out(vty,"%-4s%-12s%-5s%-7s%-8s%-7s%-7s%-6s%-3s%-15s%-6s\n",	\
				"","dest-ip","mask","hw-idx","ifindex","device",isTrunk?"trunk":"port","vlan","","mac-address","ref");
		vty_out(vty,"%-16s%-5s%-7s%-8s%-7s%-7s%-6s%-18s%-5s\n",		\
				"---------------","----","------","-------","------","------","-----","-----------------","-----");
		if(DCLI_ROUTE_RETCODE_ACTION_TRAP2CPU == op_ret) {
			vty_out(vty,"%-3d.%-3d.%-3d.%-3d %-4d %-6d %-7s %-6s %-6s %-5s %-17s %-5s\n", \
					(dip>>24)&0xFF,(dip>>16)&0xFF,(dip>>8)&0xFF,dip&0xFF,maskLen,nexthopindex, 	\
					"-","-","CPU","-","-","-");
		}
		if(DCLI_ROUTE_RETCODE_ACTION_HARD_DROP == op_ret) {
			vty_out(vty,"%-3d.%-3d.%-3d.%-3d %-4d %-6d %-7s %-6s %-6s %-5s %-17s %-5s\n", \
					(dip>>24)&0xFF,(dip>>16)&0xFF,(dip>>8)&0xFF,dip&0xFF,maskLen,nexthopindex, 	\
					"-","-","DROP","-","-","-");
		}
		if( 0 == op_ret) {
			vty_out(vty,"%-3d.%-3d.%-3d.%-3d %-4d %-6d %-7d %-6d %-6d %-5d %02X:%02X:%02X:%02X:%02X:%02X %-4d\n", \
					(dip>>24)&0xFF,(dip>>16)&0xFF,(dip>>8)&0xFF,dip&0xFF,maskLen,nexthopindex, 	\
					ifindex,devNum,portNum,vid,macAddr[0],macAddr[1],macAddr[2],macAddr[3],	\
					macAddr[4],macAddr[5],refCnt);
		}
	}

	else {
		vty_out(vty,"can't get the route entry.\n");
	}

	return;
}

static int dcli_route_config_rpf
(
	struct vty *vty,
	unsigned char rpfEn
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned char retrpfen;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,RTDRV_DBUS_OBJPATH,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_CONFIG_RPF);
		
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					 DBUS_TYPE_BYTE,&rpfEn,
					 DBUS_TYPE_INVALID);
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_BYTE, &retrpfen,
							DBUS_TYPE_INVALID)) 
	{
			vty_out(vty,"can't get the route entry msg.\n");
	}	
	dbus_message_unref(reply);
/*	
	vty_out(vty,"retrpfen = %d \n",retrpfen);
*/
	if(retrpfen!= rpfEn)
	{
		vty_out(vty,"set uc rpf failure.\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

static int dcli_route_show_rpf
(
	struct vty *vty
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned char retrpfen;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,RTDRV_DBUS_OBJPATH,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_RPF);
		
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_BYTE, &retrpfen,
							DBUS_TYPE_INVALID)) 
	{
			vty_out(vty,"can't get the route entry msg.\n");
	}	
	dbus_message_unref(reply);
	vty_out(vty,"system uc RPF is %s\n",retrpfen?"enable":"disable");

	return CMD_SUCCESS;
}

static int dcli_ip_route_fw_statistic
(
	struct vty *vty,
	unsigned char counter
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned char ret;	
	unsigned int inUcPkts;
	unsigned int inMcPkts;
	unsigned int inUcNonRoutedExcpPkts;
	unsigned int inUcNonRoutedNonExcpPkts;
	unsigned int inMcNonRoutedExcpPkts;
	unsigned int inMcNonRoutedNonExcpPkts;
	unsigned int inUcTrappedMirrorPkts;
	unsigned int inMcTrappedMirrorPkts;
	unsigned int mcRfpFailPkts;
	unsigned int outUcRoutedPkts;
	

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,RTDRV_DBUS_OBJPATH,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_STATUES);
		
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					 DBUS_TYPE_BYTE,&counter,
					 DBUS_TYPE_INVALID);
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_BYTE, &ret,
							DBUS_TYPE_UINT32, &inUcPkts,
							DBUS_TYPE_UINT32, &inMcPkts,
							DBUS_TYPE_UINT32, &inUcNonRoutedExcpPkts,
							DBUS_TYPE_UINT32, &inUcNonRoutedNonExcpPkts,
							DBUS_TYPE_UINT32, &inMcNonRoutedExcpPkts,
							DBUS_TYPE_UINT32, &inMcNonRoutedNonExcpPkts,
							DBUS_TYPE_UINT32, &inUcTrappedMirrorPkts,
							DBUS_TYPE_UINT32, &inMcTrappedMirrorPkts,
							DBUS_TYPE_UINT32, &mcRfpFailPkts,
							DBUS_TYPE_UINT32, &outUcRoutedPkts,
							DBUS_TYPE_INVALID)) 	
	{
			vty_out(vty,"can't get the route entry msg.\n");
			return CMD_WARNING;
	}	
	dbus_message_unref(reply);
	if(ret){
		return CMD_WARNING;
		}
	vty_out(vty,"inUcPkts:					%d\n",inUcPkts);
	vty_out(vty,"inMcPkts:					%d\n",inMcPkts);
	vty_out(vty,"inUcNonRoutedExcpPkts:		%d\n",inUcNonRoutedExcpPkts);
	vty_out(vty,"inUcNonRoutedNonExcpPkts:	%d\n",inUcNonRoutedNonExcpPkts);
	vty_out(vty,"inMcNonRoutedExcpPkts:		%d\n",inMcNonRoutedExcpPkts);
	vty_out(vty,"inMcNonRoutedNonExcpPkts:	%d\n",inMcNonRoutedNonExcpPkts);
	vty_out(vty,"inUcTrappedMirrorPkts:		%d\n",inUcTrappedMirrorPkts);
	vty_out(vty,"inMcTrappedMirrorPkts:		%d\n",inMcTrappedMirrorPkts);
	vty_out(vty,"mcRfpFailPkts:				%d\n",mcRfpFailPkts);
	vty_out(vty,"outUcRoutedPkts:			%d\n",outUcRoutedPkts);
	return CMD_SUCCESS;
}

//add by gjd
static int dcli_config_rpfilter(struct vty *vty, unsigned char *rpfcmd)
{
	int ret=0;
	int ret2=0;

	if( 0 == strncmp(rpfcmd,"enable",strlen(rpfcmd)))
	{
		 ret = system ("sudo sysctl -w net.ipv4.conf.all.rp_filter=1");//enable rp_filter
		 ret = WEXITSTATUS(ret);
		 ret2 = system("sudo ip route flush cache");//flush route cache
		 ret2 = WEXITSTATUS(ret2);
		 if(0!=ret)
		{
			return CMD_WARNING;
		}
		else if(0!=ret2)
		{
		  return CMD_WARNING;
		}
	  return CMD_SUCCESS;
	}
	else 
	if( 0 == strncmp(rpfcmd,"disable",strlen(rpfcmd)))
	{
		ret = system ("sudo sysctl -w net.ipv4.conf.all.rp_filter=0");//disable rp_filter
		ret = WEXITSTATUS(ret);
		if(0!=ret)
			return CMD_WARNING;
	
		return CMD_SUCCESS;
	}
}


/* for config fdb table agiingtime*/
DEFUN(show_route_mvdrv_all_cmd_func,
	show_route_mvdrv_all_cmd,
	"show drvroute",
	SHOW_STR
	"Route table infomation in MARVELL driver \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int rtentry_count,i;


	vty_out(vty,"Show  route table info on MARVELL Driver. \n");

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,RTDRV_DBUS_OBJPATH,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_ALL);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&rtentry_count);

	vty_out(vty,"%-20s%-20s%-20s%-4s%-16s\n","DIP","MASK LENGTH","NEXT HOP","INTERFACE","DMAC");
	
	
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	for (i = 0; i < rtentry_count; i++) {
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;
		unsigned int DIP,masklen,nexthopindex,ifindex;
		unsigned char	macAddr[6];
		int j;
		
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&DIP);
		/*printf("slotno %d\n",slotno);*/
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&masklen);
		/*printf("local port count %d\n",local_port_count);*/
		
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&nexthopindex);
/*
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&ifindex);

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,macAddr);
*/
		vty_out(vty,"%-20s%-20s%-20s%-4s%-16s\n",DIP,masklen,nexthopindex,ifindex,macAddr);
		
		dbus_message_iter_next(&iter_array);
		
	}
		
	dbus_message_unref(reply);
	
	
	return CMD_SUCCESS;
	
}


/* for config fdb table agiingtime*/
DEFUN(show_host_drv_all_cmd_func,
	show_host_drv_all_cmd,
	"show drvhost",
	SHOW_STR
	"Host table infomation in driver \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int rtentry_count = 0,i = 0;


	vty_out(vty,"Show  route table info on MARVELL Driver. \n");

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,RTDRV_DBUS_OBJPATH,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_HOST);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&rtentry_count);
    if(0 == rtentry_count){
        vty_out(vty,"No drive items!\n");
		dbus_message_unref(reply);	
	    return CMD_SUCCESS;
	}
	vty_out(vty,"===================ITEM %d============================\n",rtentry_count);
	vty_out(vty,"%-16s%-5s%-7s%-8s%-7s%-5s%-7s%-18s%-5s\n", 	\
			"===============","======","=======","========","=======","=====","=======","=================","=====");
	vty_out(vty,"%-4s%-12s%-7s%-8s%-10s%-8s%-5s%-3s%-15s\n", \
			"","dest-ip","mask","hw-idx","ifindex","device","port","","mac-address");
	
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	for (i = 0; i < rtentry_count; i++) {
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;
		unsigned int ip,masklen = 32,nexthopindex = 0,ifindex = 0;
		unsigned char dev = 0,port = 0;
		unsigned char	macAddr[6];
		
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&ip);
		/*printf("slotno %d\n",slotno);*/
						
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&nexthopindex);

		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&ifindex);

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dev);

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&port);

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&macAddr[0]);

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&macAddr[1]);
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&macAddr[2]);
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&macAddr[3]);
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&macAddr[4]);
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&macAddr[5]);

		vty_out(vty,"%-3d.%-3d.%-3d.%-3d %-4d %-6d %-7d %-6d %-4d %02X:%02X:%02X:%02X:%02X:%02X\n", \
				(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,ip&0xFF,masklen,nexthopindex,	\
				ifindex,dev,port,macAddr[0],macAddr[1],macAddr[2],macAddr[3], \
				macAddr[4],macAddr[5]);
		
		dbus_message_iter_next(&iter_array);
		
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}
#if 0
DEFUN(show_route_mvdrv_entry_cmd_func,
	show_route_mvdrv_entry_cmd,
	"show drvroute A.B.C.D A.B.C.D",
	SHOW_STR
	"Route table infomation on MARVELL driver \n"
	"IP address. eg:192.168.1.0\n"
	"IP mask length. eg:255.255.255.0\n"
)
{
	unsigned int DIP,masklen;

	if(argc != 2)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_SUCCESS;
	}
	if(1 != inet_atoi(argv[0],&DIP))
	{
		vty_out(vty,"can't get ip address\n");
		return CMD_SUCCESS;

	}
	masklen = get_ip_masklen(argv[1]);


	dcli_route_show_drvroute(vty,DIP,masklen);
	return CMD_SUCCESS;
}

DEFUN(show_route_mvdrv_entry1_cmd_func,
		show_route_mvdrv_entry1_cmd,
		"show drvroute A.B.C.D/M",
		SHOW_STR
		"Route table infomation on MARVELL driver \n"
		"IP address and masklen. eg:192.168.1.0/24\n"
	)
{
	unsigned int DIP,masklen;
	unsigned char ipaddrstr[17];
	int addrlen;

	if(argc != 1)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_SUCCESS;
	}
	addrlen=strchr(argv[0],'/')-argv[0];
	memcpy(ipaddrstr,argv[0],addrlen);
	ipaddrstr[addrlen]='\0';
	
	if(1 != inet_atoi(ipaddrstr,&DIP))
	{
		vty_out(vty,"can't get ip address\n");
		return CMD_SUCCESS;

	}
	masklen = atoi(argv[0]+addrlen+1);
	dcli_route_show_drvroute(vty,DIP,masklen);
	return CMD_SUCCESS;
}
#endif
DEFUN(config_rpf_cmd_func,
	config_rpf_cmd,
	"config rpf (enable|disable)",
	SETT_STR
	"Config route table rpf\n"
	"Config route table rpf enable\n"
	"Config route table rpf disable\n"
)
{
	unsigned int RPFen=0;

	if(argc != 1)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_WARNING;
	}
	if( 0 == strncmp(argv[0],"enable",strlen(argv[0])))
	{
		RPFen=1;
	}
	else if( 0 == strncmp(argv[0],"disable",strlen(argv[0])))
	{
		RPFen=0;
	}
	else
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_WARNING;
	
	}
	dcli_route_config_rpf(vty,(unsigned char)RPFen);
	dcli_config_rpfilter(vty,argv[0]);
	return CMD_SUCCESS;
}
DEFUN(show_rpf_cmd_func,
	show_rpf_cmd,
	"show rpf",
	SHOW_STR
	"Show route table rpf statues\n")
{

	if(argc != 0)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_WARNING;
	}
	dcli_route_show_rpf(vty);
	return CMD_SUCCESS;
}
DEFUN(show_statues_cmd_func,
	show_statues_cmd,
	"show ip statistic",
	SHOW_STR
	IP_STR
	"Show ip forword statistic \n")
{

	if(argc != 0)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_WARNING;
	}
	dcli_ip_route_fw_statistic(vty,0);
	return CMD_SUCCESS;
}

DEFUN(config_arp_aging_cmd_func,
	config_arp_aging_cmd,
	"config arp aging <5-60>",
	CONFIG_STR
	"ARP information\n"
	"ARP aging function\n"
	"Aging time , uses minute for unit\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned char retrpfen;
	unsigned int agingTime = 10;
	int ret = 0;

	if(argc > 1)
	{
		vty_out(vty,"command parameter error\n");
		return CMD_WARNING;
	}
	ret = parse_int_parse((unsigned char *)argv[0],&agingTime);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow  number format.\n");
		return CMD_WARNING ;
	}

	if((agingTime < 5) || (agingTime > 60)) {
		vty_out(vty,"input bad param\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(		\
											NPD_DBUS_BUSNAME,		\
											RTDRV_DBUS_OBJPATH,		\
											RTDRV_DBUS_INTERFACE,		\
											RTDRV_DBUS_METHOD_ARP_AGING_TIME);
		
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					 DBUS_TYPE_UINT32,&agingTime,
					 DBUS_TYPE_INVALID);
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS != ret ) {
				vty_out(vty,"%% config arp agine time error !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

void dcli_drv_routesyn_init() {
#if 0
	install_element(CONFIG_NODE, &show_route_mvdrv_all_cmd);
    install_element(CONFIG_NODE,&show_host_drv_all_cmd);
	install_element(CONFIG_NODE, &show_route_mvdrv_entry_cmd);
	install_element(CONFIG_NODE, &show_route_mvdrv_entry1_cmd);
#endif
	//install_element(HIDDENDEBUG_NODE, &show_route_mvdrv_entry_cmd);
	//install_element(HIDDENDEBUG_NODE, &show_route_mvdrv_entry1_cmd);	
	install_element(HIDDENDEBUG_NODE, &config_rpf_cmd);
	install_element(HIDDENDEBUG_NODE, &show_rpf_cmd);
	install_element(CONFIG_NODE, &config_rpf_cmd);
	install_element(CONFIG_NODE, &show_rpf_cmd);
/*	install_element(CONFIG_NODE, &show_rpf_cmd);*/
	install_element(HIDDENDEBUG_NODE, &show_statues_cmd);
/*	install_element(CONFIG_NODE, &show_statues_cmd);
	//install_element(CONFIG_NODE, &config_arp_aging_cmd);*/
}
#ifdef __cplusplus
}
#endif
