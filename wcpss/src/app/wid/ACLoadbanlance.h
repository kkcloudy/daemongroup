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
* ACLoadbanlance.h
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/un.h>
#include "CWAC.h"
#include "CWCommon.h"
#include "ACMultiHomedSocket.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "CWStevens.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "ACDbus_handler.h"
#include "ACDbus.h"

#define SERVPORT 9007 /*服务器监听端口号 */
#define BACKLOG 10 /* 最大同时连接请求数 */
#define MAXDATASIZE 1024

int init_client_socket();
void SendActiveWTPCount(int inum);
int get_ipv4addr_by_ifname(unsigned char ID);
void make_link_sequence_by_wtpcount(unsigned char ID);

CW_THREAD_RETURN_TYPE CWLoadbanlanceThread(void * arg);
void make_link_sequence_by_priority(unsigned char ID);


