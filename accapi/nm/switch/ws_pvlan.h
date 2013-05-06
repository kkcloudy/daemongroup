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
* ws_pvlan.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef _DCLI_PVE_H
#define _DCLI_PVE_H
#define MAX_VLANID 4094
#define MIN_VLANID 1
#define MIN_SLOT   1
#define MAX_SLOT   4
#define MIN_PORT   1
#define MAX_PORT   6
#define ENABLE     1
#define DISABLE    2
#define PVE_ERR   -1
#define MAX_TRUNK 127
#define MIN_TRUNK 1
#define MAX_PVLAN 31
#define MIN_PVLAN 1
#define PVE_MAX_PORT    64

#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'

typedef enum{
	PVE_FAIL=0,
	PVE_TRUE=1,
}PVE_BOOL;

struct Pvlan_Info
{
	unsigned int pslot;
	unsigned int pport;
	unsigned int lsort;
	unsigned int lport;
	unsigned int mode;
	unsigned int trunk;
	struct Pvlan_Info * next;
};

int is_enable_or_disable(char * str);

void dcli_pve_init(void);

extern int add_config_pvlan(char *port, char *up_port, char *create_mode);

extern int pvlan_delete(char *port);

extern int show_pvlan();

extern int set_delete_pvlan();
extern void Free_list(struct Pvlan_Info * head,int pvlannum);


#endif

