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
* ASDIappOp.h
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

#ifndef IAPP_H
#define IAPP_H

struct iapp_data;

#ifdef ASD_IAPP

void iapp_new_station(struct iapp_data *iapp, struct sta_info *sta);
struct iapp_data * iapp_init(struct asd_data *wasd, const char *iface);
void iapp_deinit(struct iapp_data *iapp);
int iapp_reconfig(struct asd_data *wasd, struct asd_config *oldconf,
		  struct asd_bss_config *oldbss);

#else /* ASD_IAPP */

static inline void iapp_new_station(struct iapp_data *iapp,
				    struct sta_info *sta)
{
}

static inline struct iapp_data * iapp_init(struct asd_data *wasd,
					   const char *iface)
{
	return NULL;
}

static inline void iapp_deinit(struct iapp_data *iapp)
{
}

static inline int
iapp_reconfig(struct asd_data *wasd, struct asd_config *oldconf,
	      struct asd_bss_config *oldbss)
{
	return 0;
}

#endif /* ASD_IAPP */

#endif /* IAPP_H */
