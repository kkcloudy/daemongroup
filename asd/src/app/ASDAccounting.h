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
* ASDAccounting.h
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

#ifndef ACCOUNTING_H
#define ACCOUNTING_H

void accounting_sta_start(struct asd_data *wasd, struct sta_info *sta);
void accounting_sta_interim(struct asd_data *wasd, struct sta_info *sta);
void accounting_sta_stop(struct asd_data *wasd, struct sta_info *sta);
void accounting_get_session_id(struct sta_info *sta);
void accounting_set_seesion_id(struct sta_info *sta);
void accounting_sta_get_id(struct asd_data *wasd, struct sta_info *sta);
int accounting_init(const int  wlanid);
void accounting_deinit(struct asd_data *wasd);
int accounting_reconfig(struct asd_data *wasd,
			struct asd_config *oldconf);

#endif /* ACCOUNTING_H */
