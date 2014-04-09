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
*dcli_sync_route_policy.c
*
* MODIFY:
*		by <gujd@autelan.com> on 2014-02-28 revision <1.0>
*
* CREATOR:
*		Gu Jindong
*            System Project Team, FSC.
*            mail: gujd@autelan.com
*
* DESCRIPTION:
*		 For Distibuted System to sync route policy config on AC product.
*
* DATE:
*		28/02/2014	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.0 $	
*******************************************************************************/

#ifndef _DCLI_SYNC_ROUTE_POLICY_H_
#define _DCLI_SYNC_ROUTE_POLICY_H_

#define RT_TABLE_DATABASE_FILE "/etc/iproute2/rt_tables"
#define RTFIELD_DATABASE_FILE  "/etc/iproute2/rt_dsfield"

#define IS_RTB_ID(_rtb_id) (_rtb_id>=1 && _rtb_id<=32)?1:0
#define RTB_NAME(_buf,_rtb_id) sprintf(_buf,"_rp%u",_rtb_id)

#define CMD_STR_LEN 256

#define rt_tables_entry_name(_rtb_id )  dcli_rp_table[_rtb_id]

struct rp_rule_node{

struct rp_rule_node *next;
struct rp_rule_node *prev;
unsigned char type;
char tablename[64];
char ifname[32];
char prefix[32];
int tos;
int sn;
};
struct rp_ip_route_node{

struct rp_ip_route_node *next;
struct rp_ip_route_node *prev;
unsigned char type;
char tablename[64];
char dst[32];
char prefix[32];
};

#endif

