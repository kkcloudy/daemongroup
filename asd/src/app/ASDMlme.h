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
* ASDMlme.h
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

#ifndef MLME_H
#define MLME_H

void mlme_authenticate_indication(struct asd_data *wasd,
				  struct sta_info *sta);

void mlme_deauthenticate_indication(struct asd_data *wasd,
				    struct sta_info *sta, u16 reason_code);

void mlme_associate_indication(struct asd_data *wasd,
			       struct sta_info *sta);

void mlme_reassociate_indication(struct asd_data *wasd,
				 struct sta_info *sta);

void mlme_disassociate_indication(struct asd_data *wasd,
				  struct sta_info *sta, u16 reason_code);

void mlme_michaelmicfailure_indication(struct asd_data *wasd,
				       const u8 *addr);

void mlme_deletekeys_request(struct asd_data *wasd, struct sta_info *sta);

#endif /* MLME_H */
