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
* dhcp_snp_main.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping main routine .
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sysdef/returncode.h>

#include "dhcp_snp_main.h"
void pid_write(char *name)
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	int myPid = 0;
	int fd;
	sprintf(pidPath,"%s%s", "/var/run/wcpss/dhcp_thread", ".pid");
		
	fd = open(pidPath, O_RDWR|O_CREAT|O_APPEND);
	if(fd<0){
		return;
	}
	myPid = getpid();	
	sprintf(pidBuf,"%s - %d\n",name, myPid);
	write(fd, pidBuf, strlen(pidBuf));
	close(fd);
	return;
}

int main()
{
	int ret = 0;
	pthread_t packet_hdlr, dbus_hdlr, aging_hdlr, asd_interactive_hdlr;
	pthread_attr_t pkt_hdlr_attr, dbus_hdlr_attr, tbl_aging_attr, asd_interactive_attr;
	
	log_info("start dhcp snooping...");
	

	ret = dhcp_snp_dbus_init();
	if(!ret) {
		log_error("dhcp snooping start dbus error %d\n", ret);
		return 1;
	}

	ret = dhcp_snp_listener_init();
	if(DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("dhcp snooping init socket error %d\n", ret);
		return 1;
	}
	
	ret = anti_arp_spoof_init();
	if(DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("anti arp spoof init error %d\n", ret);
		return 1;
	}

	ret = dhcp_snp_netlink_init();
	if(DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("dhcp snooping init netlink error %d\n", ret);
		return 1;
	}

	ret = dhcp_snp_tbl_initialize();
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("init DHCP-Snooping bind table error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	/* start packet handler */
	pthread_attr_init(&pkt_hdlr_attr);
	pthread_create(&packet_hdlr,&pkt_hdlr_attr,(void*)dhcp_snp_listener_thread_main, NULL);

	pthread_attr_init(&dbus_hdlr_attr);
	pthread_create(&dbus_hdlr,&dbus_hdlr_attr,(void*)dhcp_snp_dbus_thread_main, NULL);

	pthread_attr_init(&tbl_aging_attr);
	pthread_create(&aging_hdlr,&tbl_aging_attr,(void*)dhcp_snp_tbl_thread_aging, NULL);

	pthread_attr_init(&asd_interactive_attr);
	pthread_create(&asd_interactive_hdlr,&asd_interactive_attr,(void*)dhcp_snp_asd_interactive, NULL);


	dhcp_snp_tell_whoami("dhcpsnp main", 0);
	
	
	pthread_join(packet_hdlr, NULL);
	pthread_join(dbus_hdlr, NULL);
	pthread_join(aging_hdlr, NULL);
	pthread_join(asd_interactive_hdlr, NULL);
	
	
	return 0;
}
#ifdef __cplusplus
}
#endif
