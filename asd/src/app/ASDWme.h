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
* ASDWme.h
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

#ifndef WME_H
#define WME_H

#ifdef __linux__
#include <endian.h>
#endif /* __linux__ */

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/types.h>
#include <sys/endian.h>
#endif /* defined(__FreeBSD__) || defined(__NetBSD__) ||
	* defined(__DragonFly__) */

#define WME_OUI_TYPE 2
#define WME_OUI_SUBTYPE_INFORMATION_ELEMENT 0
#define WME_OUI_SUBTYPE_PARAMETER_ELEMENT 1
#define WME_OUI_SUBTYPE_TSPEC_ELEMENT 2
#define WME_VERSION 1

#define WME_ACTION_CATEGORY 17
#define WME_ACTION_CODE_SETUP_REQUEST 0
#define WME_ACTION_CODE_SETUP_RESPONSE 1
#define WME_ACTION_CODE_TEARDOWN 2

#define WME_SETUP_RESPONSE_STATUS_ADMISSION_ACCEPTED 0
#define WME_SETUP_RESPONSE_STATUS_INVALID_PARAMETERS 1
#define WME_SETUP_RESPONSE_STATUS_REFUSED 3

#define WME_TSPEC_DIRECTION_UPLINK 0
#define WME_TSPEC_DIRECTION_DOWNLINK 1
#define WME_TSPEC_DIRECTION_BI_DIRECTIONAL 3

extern inline u16 tsinfo(int tag1d, int contention_based, int direction)
{
	return (tag1d << 11) | (contention_based << 7) | (direction << 5) |
	  (tag1d << 1);
}


struct wme_information_element {
	/* required fields for WME version 1 */
	u8 oui[3];
	u8 oui_type;
	u8 oui_subtype;
	u8 version;
	u8 acInfo;

} __attribute__ ((packed));

struct wme_ac_parameter {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	/* byte 1 */
	u8 	aifsn:4,
		acm:1,
	 	aci:2,
	 	reserved:1;

	/* byte 2 */
	u8 	eCWmin:4,
	 	eCWmax:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	/* byte 1 */
	u8 	reserved:1,
	 	aci:2,
	 	acm:1,
	 	aifsn:4;

	/* byte 2 */
	u8 	eCWmax:4,
	 	eCWmin:4;
#else
#error	"Please fix <endian.h>"
#endif

	/* bytes 3 & 4 */
	le16 txopLimit;
} __attribute__ ((packed));

struct wme_parameter_element {
	/* required fields for WME version 1 */
	u8 oui[3];
	u8 oui_type;
	u8 oui_subtype;
	u8 version;
	u8 acInfo;
	u8 reserved;
	struct wme_ac_parameter ac[4];

} __attribute__ ((packed));

struct wme_tspec_info_element {
	u8 eid;
	u8 length;
	u8 oui[3];
	u8 oui_type;
	u8 oui_subtype;
	u8 version;
	u16 ts_info;
	u16 nominal_msdu_size;
	u16 maximum_msdu_size;
	u32 minimum_service_interval;
	u32 maximum_service_interval;
	u32 inactivity_interval;
	u32 start_time;
	u32 minimum_data_rate;
	u32 mean_data_rate;
	u32 maximum_burst_size;
	u32 minimum_phy_rate;
	u32 peak_data_rate;
	u32 delay_bound;
	u16 surplus_bandwidth_allowance;
	u16 medium_time;
} __attribute__ ((packed));


/* Access Categories */
enum {
	WME_AC_BK = 1,
	WME_AC_BE = 0,
	WME_AC_VI = 2,
	WME_AC_VO = 3
};

struct ieee80211_mgmt;

u8 * asd_eid_wme(struct asd_data *wasd, u8 *eid);
int asd_eid_wme_valid(struct asd_data *wasd, u8 *eid, size_t len);
int asd_wme_sta_config(struct asd_data *wasd, struct sta_info *sta);
void asd_wme_action(struct asd_data *wasd, struct ieee80211_mgmt *mgmt,
			size_t len);

#endif /* WME_H */
