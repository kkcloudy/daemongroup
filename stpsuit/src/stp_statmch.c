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
* stp_statmch.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Generic (abstract state machine) state machine in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* Generic (abstract) state machine : 17.13, 17.14 */
 
#include "stp_base.h"
#include "stp_statmch.h"
#include "stp_log.h"

#if STP_DBG
#include "stp_stpm.h"
#endif

extern int stp_log_level;

STATE_MACH_T *
stp_statmch_create (void (*concreteEnterState) (STATE_MACH_T*),
                       Bool (*concreteCheckCondition) (STATE_MACH_T*),
                       char *(*concreteGetStatName) (int),
                       void *owner, char *name)
{
  STATE_MACH_T *this;

  STP_MALLOC(this, STATE_MACH_T, "state machine");
 
  this->State = BEGIN;
  memcpy(this->name, (char*) strdup (name), 32);   /*mstp*/
  /*this->name = (char*) strdup (name);*/
  this->changeState = False;
#if STP_DBG
  this->debug = False;
  this->ignoreHop2State = BEGIN;
#endif
  this->concreteEnterState = concreteEnterState;
  this->concreteCheckCondition = concreteCheckCondition;
  this->concreteGetStatName = concreteGetStatName;
  this->owner.owner = owner;

  return this;
}
                              
void
stp_statmch_delete (STATE_MACH_T *this)
{
  /*free (this->name);*/
  STP_FREE(this, "state machine");
}

Bool
stp_statmch_check_condition (STATE_MACH_T* this)
{
  Bool bret;

  bret = (*(this->concreteCheckCondition)) (this);
  if (bret) {
    this->changeState = True;
  }
  
  return bret;
}
        
Bool
stp_statmch_change_state (STATE_MACH_T* this)
{
  register int number_of_loops;

  for (number_of_loops = 0; ; number_of_loops++) {
    if (! this->changeState) return number_of_loops;
    (*(this->concreteEnterState)) (this);
    this->changeState = False;
    stp_statmch_check_condition (this);
  }

  return number_of_loops;
}

Bool
stp_statmch_hop_2_state (STATE_MACH_T* this, unsigned int new_state)
{

 // switch (this->debug) {
    //case 0: break;
   // case 1:
   if(stp_log_level & STP_LOG_PROTOCOL)
   	{
	     if (new_state == this->State || new_state == this->ignoreHop2State);
		 else {
			    if((0 != strlen(this->name))&&(NULL != this->owner.port)&&(0 != strlen( this->owner.port->port_name)))
                /*
				 stp_trace ("%-8s(%s-%s): %s=>%s",
			        this->name,
			        *this->owner.port->owner->name ? this->owner.port->owner->name : "Glbl",
			        this->owner.port->port_name,
			        (*(this->concreteGetStatName)) (this->State),
			        (*(this->concreteGetStatName)) (new_state));
			        */
			     stp_syslog_protocol("%-8s(%s): %s=>%s\n",
			        this->name,
			        this->owner.port->port_name,
			        (*(this->concreteGetStatName))(this->State),
			        (*(this->concreteGetStatName))(new_state));
		}

		/*case 2:
		if (new_state == this->State) break;
		stp_trace ("%s(%s): %s=>%s", 
		  this->name,
		  *this->owner.stpm->name ? this->owner.stpm->name : "Glbl",
		  (*(this->concreteGetStatName)) (this->State),
		  (*(this->concreteGetStatName)) (new_state));
		break;*/
  }


  this->State = new_state;
  this->changeState = True;
  return True;
}
#ifdef __cplusplus
}
#endif

