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
* AsdHwFeatures.c
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

#include "includes.h"

#include "asd.h"
#include "ASDHWInfo.h"
#include "ASDCallback.h"
#include "config.h"
#include "circle.h"


void asd_free_hw_features(struct asd_hw_modes *hw_features,
			      size_t num_hw_features)
{
	size_t i;

	if (hw_features == NULL)
		return;

	for (i = 0; i < num_hw_features; i++) {
		os_free(hw_features[i].channels);
		hw_features[i].channels=NULL;
		os_free(hw_features[i].rates);
		hw_features[i].rates=NULL;
	}

	os_free(hw_features);
	hw_features=NULL;
}


int asd_get_hw_features(struct asd_iface *iface)
{
	struct asd_data *wasd = iface->bss[0];
	int ret = 0, i, j;
	u16 num_modes, flags;
	struct asd_hw_modes *modes;

	modes = asd_get_hw_feature_data(wasd, &num_modes, &flags);
	if (modes == NULL) {
		asd_logger(wasd, NULL, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "Fetching hardware channel/rate support not "
			       "supported.");
		return -1;
	}

	iface->hw_flags = flags;

	asd_free_hw_features(iface->hw_features, iface->num_hw_features);
	iface->hw_features = modes;
	iface->num_hw_features = num_modes;

	for (i = 0; i < num_modes; i++) {
		struct asd_hw_modes *feature = &modes[i];
		/* set flag for channels we can use in current regulatory
		 * domain */
		for (j = 0; j < feature->num_channels; j++) {
			/* TODO: add regulatory domain lookup */
			unsigned char power_level = 0;
			unsigned char antenna_max = 0;

			if ((feature->mode == asd_MODE_IEEE80211G ||
			     feature->mode == asd_MODE_IEEE80211B) &&
			    feature->channels[j].chan >= 1 &&
			    feature->channels[j].chan <= 11) {
				power_level = 20;
				feature->channels[j].flag |=
					asd_CHAN_W_SCAN;
			} else
				feature->channels[j].flag &=
					~asd_CHAN_W_SCAN;

			asd_set_channel_flag(wasd, feature->mode,
						 feature->channels[j].chan,
						 feature->channels[j].flag,
						 power_level,
						 antenna_max);
		}
	}

	return ret;
}


static int asd_prepare_rates(struct asd_data *wasd,
				 struct asd_hw_modes *mode)
{
	int i, num_basic_rates = 0;
	int basic_rates_a[] = { 60, 120, 240, -1 };
	int basic_rates_b[] = { 10, 20, -1 };
	int basic_rates_g[] = { 10, 20, 55, 110, -1 };
	int *basic_rates;

	if (wasd->iconf->basic_rates)
		basic_rates = wasd->iconf->basic_rates;
	else switch (mode->mode) {
	case asd_MODE_IEEE80211A:
		basic_rates = basic_rates_a;
		break;
	case asd_MODE_IEEE80211B:
		basic_rates = basic_rates_b;
		break;
	case asd_MODE_IEEE80211G:
		basic_rates = basic_rates_g;
		break;
	default:
		return -1;
	}

	if (asd_set_rate_sets(wasd, wasd->iconf->supported_rates,
				  basic_rates, mode->mode)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to update rate sets in kernel "
			   "module");
	}

	os_free(wasd->iface->current_rates);
	wasd->iface->num_rates = 0;

	wasd->iface->current_rates =
		os_zalloc(mode->num_rates * sizeof(struct asd_rate_data));
	if (!wasd->iface->current_rates) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to allocate memory for rate "
			   "table.");
		return -1;
	}

	for (i = 0; i < mode->num_rates; i++) {
		struct asd_rate_data *rate;

		if (wasd->iconf->supported_rates &&
		    !asd_rate_found(wasd->iconf->supported_rates,
					mode->rates[i].rate))
			continue;

		rate = &wasd->iface->current_rates[wasd->iface->num_rates];
		os_memcpy(rate, &mode->rates[i],
			  sizeof(struct asd_rate_data));
		if (asd_rate_found(basic_rates, rate->rate)) {
			rate->flags |= asd_RATE_BASIC;
			num_basic_rates++;
		} else
			rate->flags &= ~asd_RATE_BASIC;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RATE[%d] rate=%d flags=0x%x",
			   wasd->iface->num_rates, rate->rate, rate->flags);
		wasd->iface->num_rates++;
	}

	if (wasd->iface->num_rates == 0 || num_basic_rates == 0) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "No rates remaining in supported/basic "
			   "rate sets (%d,%d).",
			   wasd->iface->num_rates, num_basic_rates);
		return -1;
	}

	return 0;
}


static void select_hw_mode_start(void *circle_data, void *user_ctx);
static void select_hw_mode2_handler(void *circle_data, void *user_ctx);

/**
 * select_hw_mode_finalize - Finish select HW mode & call the callback
 * @iface: Pointer to interface data.
 * @status: Status of the select HW mode (0 on success; -1 on failure).
 * Returns: 0 on success; -1 on failure (e.g., was not in progress).
 */
static int select_hw_mode_finalize(struct asd_iface *iface, int status)
{
	asd_iface_cb cb;

	if (!iface->hw_mode_sel_cb)
		return -1;

	circle_cancel_timeout(select_hw_mode_start, iface, NULL);
	circle_cancel_timeout(select_hw_mode2_handler, iface, NULL);

	cb = iface->hw_mode_sel_cb;

	iface->hw_mode_sel_cb = NULL;

	cb(iface, status);

	return 0;
}


/**
 * select_hw_mode2 - Select the hardware mode (part 2)
 * @iface: Pointer to interface data.
 * @status: Status of auto chanel selection.
 *
 * Setup the rates and passive scanning based on the configuration.
 */
static void select_hw_mode2(struct asd_iface *iface, int status)
{
	int ret = status;
	if (ret)
		goto fail;

	if (iface->current_mode == NULL) {
		asd_logger(iface->bss[0], NULL, asd_MODULE_IEEE80211,
			       asd_LEVEL_WARNING,
			       "Hardware does not support configured channel");
		ret = -1;
		goto fail;
	}

	if (asd_prepare_rates(iface->bss[0], iface->current_mode)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to prepare rates table.");
		asd_logger(iface->bss[0], NULL, asd_MODULE_IEEE80211,
					   asd_LEVEL_WARNING,
					   "Failed to prepare rates table.");
		ret = -1;
		goto fail;
	}

	ret = asd_passive_scan(iface->bss[0], 0,
				   iface->conf->passive_scan_mode,
				   iface->conf->passive_scan_interval,
				   iface->conf->passive_scan_listen,
				   NULL, NULL);
	if (ret) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not set passive scanning: %s",
			   strerror(ret));
		ret = 0;
	}

fail:
	select_hw_mode_finalize(iface, ret);
}


/**
 * select_hw_mode2_handler - Calls select_hw_mode2 when auto chan isn't used
 * @circle_data: Stores the struct asd_iface * for the interface.
 * @user_ctx: Unused.
 */
static void select_hw_mode2_handler(void *circle_data, void *user_ctx)
{
	struct asd_iface *iface = circle_data;

	select_hw_mode2(iface, 0);
}


/**
 * select_hw_mode1 - Select the hardware mode (part 1)
 * @iface: Pointer to interface data.
 * Returns: 0 on success; -1 on failure.
 *
 * Setup the hardware mode and channel based on the configuration.
 * Schedules select_hw_mode2() to be called immediately or after automatic
 * channel selection takes place.
 */
static int select_hw_mode1(struct asd_iface *iface)
{
	int i, j, ok;

	if (iface->num_hw_features < 1)
		return -1;

	iface->current_mode = NULL;
	for (i = 0; i < iface->num_hw_features; i++) {
		struct asd_hw_modes *mode = &iface->hw_features[i];
		if (mode->mode == (int) iface->conf->hw_mode) {
			iface->current_mode = mode;
			break;
		}
	}

	if (iface->current_mode == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Hardware does not support configured "
			   "mode");
		asd_logger(iface->bss[0], NULL, asd_MODULE_IEEE80211,
			       asd_LEVEL_WARNING,
			       "Hardware does not support configured mode "
			       "(%d)", (int) iface->conf->hw_mode);
		return -1;
	}

	ok = 0;
	for (j = 0; j < iface->current_mode->num_channels; j++) {
		struct asd_channel_data *chan =
			&iface->current_mode->channels[j];
		if ((chan->flag & asd_CHAN_W_SCAN) &&
		    (chan->chan == iface->conf->channel)) {
			ok = 1;
			break;
		}
	}
	if (ok == 0 && iface->conf->channel != 0) {
		asd_logger(iface->bss[0], NULL,
			       asd_MODULE_IEEE80211,
			       asd_LEVEL_WARNING,
			       "Configured channel (%d) not found from the "
			       "channel list of current mode (%d) %s",
			       iface->conf->channel,
			       iface->current_mode->mode,
			       asd_hw_mode_txt(iface->current_mode->mode));
		iface->current_mode = NULL;
	}

	/*
	 * Calls select_hw_mode2() via a handler, so that the function is
	 * always executed from circle.
	 */
	circle_register_timeout(0, 0, select_hw_mode2_handler, iface, NULL);
	return 0;
}


/**
 * select_hw_mode_start - Handler to start select HW mode
 * @circle_data: Stores the struct asd_iface * for the interface.
 * @user_ctx: Unused.
 *
 * An circle handler is used so that all errors can be processed by the
 * callback without introducing stack recursion.
 */
static void select_hw_mode_start(void *circle_data, void *user_ctx)
{
	struct asd_iface *iface = (struct asd_iface *)circle_data;

	int ret;

	ret = select_hw_mode1(iface);
	if (ret)
		select_hw_mode_finalize(iface, ret);
}


/**
 * asd_select_hw_mode_start - Start selection of the hardware mode
 * @iface: Pointer to interface data.
 * @cb: The function to callback when done.
 * Returns:  0 if it starts successfully; cb will be called when done.
 *          -1 on failure; cb will not be called.
 *
 * Sets up the hardware mode, channel, rates, and passive scanning
 * based on the configuration.
 */
int asd_select_hw_mode_start(struct asd_iface *iface,
				 asd_iface_cb cb)
{
	if (iface->hw_mode_sel_cb) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,
			   "%s: Hardware mode select already in progress.",
			   iface->bss[0]->conf->iface);
		return -1;
	}

	iface->hw_mode_sel_cb = cb;

	circle_register_timeout(0, 0, select_hw_mode_start, iface, NULL);

	return 0;
}


/**
 * asd_auto_chan_select_stop - Stops automatic channel selection
 * @iface: Pointer to interface data.
 * Returns:  0 if successfully stopped;
 *          -1 on failure (i.e., was not in progress)
 */
int asd_select_hw_mode_stop(struct asd_iface *iface)
{
	return select_hw_mode_finalize(iface, -1);
}


const char * asd_hw_mode_txt(int mode)
{
	switch (mode) {
	case asd_MODE_IEEE80211A:
		return "IEEE 802.11a";
	case asd_MODE_IEEE80211B:
		return "IEEE 802.11b";
	case asd_MODE_IEEE80211G:
		return "IEEE 802.11g";
	default:
		return "UNKNOWN";
	}
}


int asd_hw_get_freq(struct asd_data *wasd, int chan)
{
	int i;

	if (!wasd->iface->current_mode)
		return 0;

	for (i = 0; i < wasd->iface->current_mode->num_channels; i++) {
		struct asd_channel_data *ch =
			&wasd->iface->current_mode->channels[i];
		if (ch->chan == chan)
			return ch->freq;
	}

	return 0;
}


int asd_hw_get_channel(struct asd_data *wasd, int freq)
{
	int i;

	if (!wasd->iface->current_mode)
		return 0;

	for (i = 0; i < wasd->iface->current_mode->num_channels; i++) {
		struct asd_channel_data *ch =
			&wasd->iface->current_mode->channels[i];
		if (ch->freq == freq)
			return ch->chan;
	}

	return 0;
}
