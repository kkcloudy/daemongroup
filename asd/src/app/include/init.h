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
* Init.h
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

#include "auth.h"
#define COMMTYPE_GROUP	8
#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_SYS_DEV_WIFI	"/proc/net/dev"

int ap_initialize_wie(apdata_info *papdata);

int ap_initialize_alg(apdata_info *papdata);

int ap_initialize(apdata_info *pap);

void ap_initialize_sta_table(apdata_info *pap);

/*初始化鉴别与密钥管理套间*/
int ap_initialize_akm(apdata_info *apdata);

int set_wapi(apdata_info *pap);

int check_file_valid(char *filename);
 int open_socket_for_asu() ;
int socket_open_for_ioctl();

int socket_open_for_netlink();
int get_wlandev(char *name, int len);
void free_all(struct asd_wapi *tmp_circle);
void free_one_interface(struct wapid_interfaces *interfaces );
void free_one_wapi(struct asd_wapi * wapi_wasd);

