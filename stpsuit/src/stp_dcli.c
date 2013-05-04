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
* stp_dcli.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for communication interface with CLI in stp module
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "stp_cli.h"
#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_dcli.h"
#include "stp_bitmap.h"
#include "stp_uid.h"
#include "stp_in.h"
#include "stp_to.h"

int I_am_a_stupid_hub = 0;
extern UID_STP_MODE_T g_stp_state;

#if 0
static void stp_dcli_out_port_id (int port, unsigned char cr)
{
  printf ("%s", stp_to_get_port_name (port));
  if (cr)
        printf("\n");
}
#endif
static int stp_dcli_enable (int argc, char** argv)
{
  UID_STP_CFG_T uid_cfg;
  int rc;
  STPM_T *stpm;

  g_stp_state = STP_ENABLED;
 for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next)
 {
    uid_cfg.field_mask = BR_CFG_STATE;
    uid_cfg.stp_enabled = STP_ENABLED;
    rc = stp_in_stpm_set_cfg (stpm,  &uid_cfg);
    if (rc) {
      printf ("can't enable: %s\n", stp_in_get_error_explanation (rc));
    } else
      I_am_a_stupid_hub = 0;
  }
  return 0;
}

static int stp_dcli_disable (int argc, char** argv)
{
  UID_STP_CFG_T uid_cfg;
  int rc;
  STPM_T *stpm;

  g_stp_state = STP_DISABLED;
 for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next)
 {
    uid_cfg.field_mask = BR_CFG_STATE;
    uid_cfg.stp_enabled = STP_DISABLED;
    rc = stp_in_stpm_set_cfg (stpm, &uid_cfg);
    if (rc) {
      printf ("can't disable: %s\n", stp_in_get_error_explanation (rc));
    } else
      I_am_a_stupid_hub = 1;
  }
  return 0;
}

static int stp_dcli_br_get_cfg (int argc, char** argv)
{
  long value;
  int iret = 0;

  value = strtoul(argv[1], 0, 10);
  if(value > STP_MSTI_MAX)
    return -1;
  iret = stp_mgmt_stpm_state_show (value);
  if(iret)
  	printf("instance %d is not exist\n",value);
  return 0;
}

static void
stp_dcli_show_rstp_port (BITMAP_T* ports_bitmap, int mstid)
{
  PORT_T *port;
  int port_index;
  

  for (port_index = 0; port_index <= NUMBER_OF_PORTS; port_index++) {
    if (! stp_bitmap_get_bit(ports_bitmap, port_index - 1)) continue;
    port = stp_port_mst_findport(mstid, port_index);
    if(!port)
      printf("can not get instance %d port %d.\n", mstid, port_index);
    stp_mgmt_port_state_show(port, 1);
  }
}

static int stp_dcli_pr_get_cfg (int argc, char** argv)
{
  BITMAP_T        ports_bitmap;
  int             port_index;
  int        mstid;

  if ('a' == argv[1][0]) {
    stp_bitmap_set_allbits(&ports_bitmap);
    mstid = 0;
  } else {
    port_index = strtoul(argv[2], 0, 10);
    stp_bitmap_clear(&ports_bitmap);
    stp_bitmap_set_bit(&ports_bitmap, port_index - 1);
    mstid = strtoul(argv[1], 0, 10);;
  }

  stp_dcli_show_rstp_port (&ports_bitmap, mstid);

  return 0;
}

static void
stp_dcli_set_bridge_cfg_value (unsigned short mstid, unsigned long value, unsigned long val_mask)
{
  UID_STP_CFG_T uid_cfg;
  char*         val_name;
  int           rc;
  STPM_T *stpm;

  uid_cfg.field_mask = val_mask;
  switch (val_mask) {
    case BR_CFG_STATE:
      uid_cfg.stp_enabled = value;
      val_name = "state";
      break;
    case BR_CFG_PRIO:
      uid_cfg.bridge_priority = value;
      val_name = "priority";
      break;
    case BR_CFG_AGE:
      uid_cfg.max_age = value;
      val_name = "max_age";
      break;
    case BR_CFG_HELLO:
      uid_cfg.hello_time = value;
      val_name = "hello_time";
      break;
    case BR_CFG_DELAY:
      uid_cfg.forward_delay = value;
      val_name = "forward_delay";
      break;
    case BR_CFG_FORCE_VER:
      uid_cfg.force_version = value;
      val_name = "force_version";
      break;
    case BR_CFG_MAX_HOPS:
      uid_cfg.maxhops = value;
      val_name = "max_hops";
      break;		
    case BR_CFG_AGE_MODE:
    case BR_CFG_AGE_TIME:
    default: printf ("Invalid value mask 0X%lx\n", val_mask);  return;
      break;
  }

  if(BR_CFG_MAX_HOPS == val_mask)
  {
    for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next)
    {
        rc = stp_in_stpm_set_cfg (stpm,  &uid_cfg);
        if (0 != rc) {
          printf ("Can't change rstp bridge %s:%s", val_name, stp_in_get_error_explanation (rc));
        }
    }
  }
  else
  {
     stpm = stp_stpm_get_instance(mstid);
     rc = STP_Vlan_Had_Not_Yet_Been_Created;
     if(stpm)
       rc = stp_in_stpm_set_cfg (stpm,  &uid_cfg);
  }
  if (0 != rc) {
    printf ("Can't change rstp bridge %s:%s", val_name, stp_in_get_error_explanation (rc));
  } else {
    printf ("Changed rstp bridge %s\n", val_name);
  }
}
static int stp_dcli_br_name (int argc, char** argv)
{
  STPM_T *stpm;

  if (strlen (argv[1]) > 32 ) {
    printf("the name is too long, please limit it in 32 char.");
    return 0;
  } 

  if (argv[1][0] == 0) {
    printf ("the name is illegal.\n");
    return 0;
  }
  stpm = stp_stpm_get_the_cist();
  if(stpm)
  {
      memset(stpm->MstConfigId.ConfigurationName, 0, 32);
      memcpy(stpm->MstConfigId.ConfigurationName, argv[1], strlen(argv[1]));
  }

  return 0;
}

static int stp_dcli_br_revision (int argc, char** argv)
{
  short br_revision =0;   
  STPM_T *stpm;

  br_revision = strtoul(argv[1], 0, 10); 
  stpm = stp_stpm_get_the_cist();
  if(stpm)
  {
      *((short *)stpm->MstConfigId.RevisionLevel) = br_revision;
  }

  return 0;
}


static int stp_dcli_br_prio (int argc, char** argv)
{
  long      br_prio = 32768L;
  int        mstid;

  mstid = strtoul(argv[1], 0, 10);
  br_prio = strtoul(argv[2], 0, 10);

  if (! br_prio) {
    printf ("Warning: newPriority=0, are you sure ?\n");
  }

  stp_dcli_set_bridge_cfg_value (mstid, br_prio, BR_CFG_PRIO);

  return 0;
}

static int stp_dcli_br_maxage (int argc, char** argv)
{
  long      value = 20L;

  value = strtoul(argv[1], 0, 10);
  stp_dcli_set_bridge_cfg_value (0, value, BR_CFG_AGE);
  return 0;
}

static int stp_dcli_br_fdelay (int argc, char** argv)
{
  long      value = 15L;

  value = strtoul(argv[1], 0, 10);
  stp_dcli_set_bridge_cfg_value (0, value, BR_CFG_DELAY);
  return 0;
}

static int stp_dcli_br_maxhops (int argc, char** argv)
{
  long      value = 20L;

  value = strtoul(argv[1], 0, 10);
  stp_dcli_set_bridge_cfg_value (0, value, BR_CFG_MAX_HOPS);
  return 0;
}
static int stp_dcli_br_fvers (int argc, char** argv)
{
  long      value = 2L;

  switch (argv[1][0]) {
      case '0':
      case '1':
      case 'f':
      case 'F':
        value = 0L;
        printf ("Accepted 'force_slow'\n");
        break;
      case '2':
      case 'r':
      case 'R':
        printf ("Accepted 'rapid'\n");
        value = 2L;
        break;
      default:
        printf ("Invalid argument '%s'\n", argv[1]);
        return 0;
  }
  
  stp_dcli_set_bridge_cfg_value (0, value, BR_CFG_FORCE_VER);
  return 0;
}

static void
stp_dcli_set_port_cfg_value (unsigned short mstid,
                         int port_index,
                         unsigned long value,
                         unsigned long val_mask)
{
  UID_STP_PORT_CFG_T uid_cfg;
  int           rc;
  char          *val_name;
  PORT_T *port;
  int port_no;

  uid_cfg.field_mask = val_mask;
  switch (val_mask) {
    case PT_CFG_MCHECK:
      val_name = "mcheck";
      break;
    case PT_CFG_COST:
      uid_cfg.admin_port_path_cost = value;
      val_name = "path cost";
      break;
    case PT_CFG_PRIO:
      uid_cfg.port_priority = value;
      val_name = "priority";
      break;
    case PT_CFG_P2P:
      uid_cfg.admin_point2point = (ADMIN_P2P_T) value;
      val_name = "p2p flag";
      break;
    case PT_CFG_EDGE:
      uid_cfg.admin_edge = value;
      val_name = "adminEdge";
      break;
    case PT_CFG_NON_STP:
      uid_cfg.admin_non_stp = value;
      val_name = "adminNonStp";
      break;
#ifdef STP_DBG
    case PT_CFG_DBG_SKIP_TX:
      uid_cfg.skip_tx = value;
      val_name = "skip tx";
      break;
    case PT_CFG_DBG_SKIP_RX:
      uid_cfg.skip_rx = value;
      val_name = "skip rx";
      break;
#endif
    case PT_CFG_STATE:
    default:
      printf ("Invalid value mask 0X%lx\n", val_mask);
      return;
  }
  if (port_index > 0) {
    port = stp_port_mst_findport(mstid, port_index);
    rc = STP_ERROR;
    if(port)
    {
      rc = stp_in_set_port_cfg (port, &uid_cfg);
      if(PT_CFG_EDGE == val_mask || PT_CFG_NON_STP == val_mask)
      {
        PORT_T *mstport;
        for(mstport = port->nextMst; mstport; mstport = mstport->nextMst)
	   rc = stp_in_set_port_cfg (mstport, &uid_cfg);	
      }
    }
  } else {
      for (port_no = 1; port_no <= max_port; port_no++) {
          port = stp_port_mst_findport(mstid, port_no);
          if(port)
          {
            rc = stp_in_set_port_cfg (port, &uid_cfg);
            if(PT_CFG_EDGE == val_mask || PT_CFG_NON_STP == val_mask)
            {
              PORT_T *mstport;
              for(mstport = port->nextMst; mstport; mstport = mstport->nextMst)
                rc = stp_in_set_port_cfg (mstport, &uid_cfg);	
            }
          }
      }
  }  
  if (0 != rc) {
    printf ("can't change rstp port[%s] %s: %s\n",
           val_name, stp_in_get_error_explanation (rc));
  } else {
    printf ("changed rstp port[%s] \n", val_name);
  }

}

static int stp_dcli_prt_prio (int argc, char** argv)
{
  int port_index = 0;
  unsigned long value = 128;
  int mstid = 0;

  mstid = strtoul(argv[1], 0, 10);

  if ('a' != argv[2][0])
    port_index = strtoul(argv[2], 0, 10);

  value = strtoul(argv[3], 0, 10);
  stp_dcli_set_port_cfg_value (mstid, port_index, value, PT_CFG_PRIO);
  return 0;
}

static int stp_dcli_prt_pcost (int argc, char** argv)
{
  int port_index = 0;
  unsigned long value = 0;
  int mstid = 0;

  mstid = strtoul(argv[1], 0, 10);
  
  if ('a' != argv[2][0])
    port_index = strtoul(argv[2], 0, 10);

  value = strtoul(argv[3], 0, 10);
  stp_dcli_set_port_cfg_value (mstid, port_index, value, PT_CFG_COST);
  return 0;
}

static int stp_dcli_prt_mcheck (int argc, char** argv)
{
  int port_index = 0;
  int mstid = 0;

  mstid = strtoul(argv[1], 0, 10);

  if ('a' != argv[2][0])
    port_index = strtoul(argv[2], 0, 10);
  stp_dcli_set_port_cfg_value (mstid, port_index, 0, PT_CFG_MCHECK);
  return 0;
}

static int stp_dcli_get_bool_arg (int narg, int argc, char** argv,
                         unsigned long* value)
{
  switch (argv[narg][0]) {
    case 'y':
    case 'Y':
      *value = 1;
      break;
    case 'n':
    case 'N':
      *value = 0;
      break;
    default:
      printf ("Invalid Bollean parameter '%s'\n", argv[narg]);
      return -1;
  }
  return 0;
}

static int stp_dcli_prt_edge (int argc, char** argv)
{
  int port_index = 0;
  unsigned long value = 1;
  int mstid = 0;

  if ('a' != argv[1][0])
    port_index = strtoul(argv[1], 0, 10);

  if (0 != stp_dcli_get_bool_arg (2, argc, argv, &value))
    return 0;

  stp_dcli_set_port_cfg_value (mstid, port_index, value, PT_CFG_EDGE);
  return 0;
}

static int stp_dcli_prt_non_stp (int argc, char** argv)
{
  int port_index = 0;
  unsigned long value = 0;
  int mstid = 0;

  if ('a' != argv[1][0])
    port_index = strtoul(argv[1], 0, 10);

  if (0 != stp_dcli_get_bool_arg (2, argc, argv, &value))
    return 0;

  stp_dcli_set_port_cfg_value (mstid, port_index, value, PT_CFG_NON_STP);
  return 0;
}

static int stp_dcli_prt_p2p (int argc, char** argv)
{
  int port_index = 0;
  unsigned long value = P2P_FORCE_TRUE_E;
  int mstid = 0;

  mstid = strtoul(argv[1], 0, 10);

  if ('a' != argv[2][0])
    port_index = strtoul(argv[2], 0, 10);

  switch (argv[3][0]) {
      case 'y':
      case 'Y':
        value = P2P_FORCE_TRUE_E;
        break;
      case 'n':
      case 'N':
        value = P2P_FORCE_FALSE_E;
        break;
      case 'a':
      case 'A':
        value = P2P_AUTO_E;
        break;
      default:
        printf ("Invalid parameter '%s'\n", argv[3]);
        return 0;
  }

  stp_dcli_set_port_cfg_value (mstid, port_index, (ADMIN_P2P_T) value, PT_CFG_P2P);
  return 0;
}

static int stp_dcli_set_vlan_to_instance (int argc, char** argv)
{
  int vlan_id;
  int mstid ;

  vlan_id = strtoul(argv[2], 0, 10);
  mstid = strtoul(argv[1], 0, 10);
  stp_in_set_vlan_2_instance (vlan_id, mstid);
  return 0;
}

#ifdef STP_DBG
static int stp_dcli_trace (int argc, char** argv)
{
  BITMAP_T ports_bitmap;
  int port_index;
  int mstid ;

  if ('a' == argv[2][0]) {
    stp_bitmap_set_allbits(&ports_bitmap);
  } else {
    port_index = strtoul(argv[2], 0, 10);
    mstid = strtoul(argv[1], 0, 10);
    stp_bitmap_clear(&ports_bitmap);
    stp_bitmap_set_bit(&ports_bitmap, port_index - 1);
  }

  stp_in_dbg_set_port_trace (argv[3],
                             argv[4][0] != 'n' && argv[4][0] != 'N',
                             0, &ports_bitmap, mstid,
                             1);
  return 0;
}

/****
  PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
  PARAM_ENUM("receive or/and transmit")
    PARAM_ENUM_SEL("rx", "receive")
    PARAM_ENUM_SEL("tx", "transmit")
    PARAM_ENUM_DEFAULT("all")
  PARAM_NUMBER("number of BPDU to skip", 0, 10000, "1")
****/
static int stp_dcli_skip (int argc, char** argv)
{
#if 0
  int port_index = 0, to_skip;

  if ('a' != argv[1][0])
    port_index = strtoul(argv[1], 0, 10);

  to_skip = atoi (argv[3]);

  if ('a' == argv[2][0] || 'r' == argv[2][0]) {
    stp_dcli_set_port_cfg_value (port_index, to_skip, PT_CFG_DBG_SKIP_RX);
  }

  if ('a' == argv[2][0] || 't' == argv[2][0]) {
    stp_dcli_set_port_cfg_value (port_index, to_skip, PT_CFG_DBG_SKIP_TX);
  }
#endif
  return 0;
}

static int stp_dcli_sleep (int argc, char** argv)
{
  int delay = atoi (argv[1]);
  sleep (delay);
  return 0;
}

#endif

static CMD_DSCR_T lang[] = {
	THE_COMMAND("enable", "enable mstp")
	THE_FUNC(stp_dcli_enable)

	THE_COMMAND("disable", "disable mstp")
	THE_FUNC(stp_dcli_disable)

	THE_COMMAND("show instance", "show  the instance's state")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	THE_FUNC(stp_dcli_br_get_cfg)

	THE_COMMAND("show port", "get port config")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	PARAM_NUMBER("port number on bridge", 1, NUMBER_OF_PORTS, "all")
	THE_FUNC(stp_dcli_pr_get_cfg)

	THE_COMMAND("region name", "set region name")
	PARAM_STRING("name", "mstplib1.0")
	THE_FUNC(stp_dcli_br_name)

	THE_COMMAND("bridge revision", "set bridge revision")
	PARAM_NUMBER("revision", MIN_BR_PRIO, MAX_BR_PRIO, "0x8000")
	THE_FUNC(stp_dcli_br_revision)

	THE_COMMAND("bridge priority", "set bridge priority")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	PARAM_NUMBER("priority", MIN_BR_PRIO, MAX_BR_PRIO, "0x8000")
	THE_FUNC(stp_dcli_br_prio)

	THE_COMMAND("bridge maxage", "set bridge maxAge")
	PARAM_NUMBER("maxAge", MIN_BR_MAXAGE, MAX_BR_MAXAGE, "20")
	THE_FUNC(stp_dcli_br_maxage)

	THE_COMMAND("bridge fdelay", "set bridge forwardDelay")
	PARAM_NUMBER("forwardDelay", MIN_BR_FWDELAY, MAX_BR_FWDELAY, "15")
	THE_FUNC(stp_dcli_br_fdelay)

	THE_COMMAND("bridge forseVersion", "set bridge forseVersion")
	PARAM_BOOL("forseVersion", "forse slow", "regular", "no")
	THE_FUNC(stp_dcli_br_fvers)

	THE_COMMAND("bridge maxhops", "set bridge maxAge")
	PARAM_NUMBER("maxHops", MIN_REMAINING_HOPS, MAX_REMAINING_HOPS, "20")
	THE_FUNC(stp_dcli_br_maxhops)


	THE_COMMAND("port priority", "set port priority")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	PARAM_NUMBER("priority", MIN_PORT_PRIO, MAX_PORT_PRIO, "128")
	THE_FUNC(stp_dcli_prt_prio)

	THE_COMMAND("port pcost", "set port path cost")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	PARAM_NUMBER("path cost (0- for auto)", 0, 200000000, 0)
	THE_FUNC(stp_dcli_prt_pcost)

	THE_COMMAND("port mcheck", "set port mcheck")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	THE_FUNC(stp_dcli_prt_mcheck)

	THE_COMMAND("port edge", "set port adminEdge")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	PARAM_BOOL("adminEdge", "Edge", "noEdge", "Y")
	THE_FUNC(stp_dcli_prt_edge)

	THE_COMMAND("port nonStp", "set port adminNonStp")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	PARAM_BOOL("adminnonStp", "Doesn't participate", "Paricipates", "n")
	THE_FUNC(stp_dcli_prt_non_stp)

	THE_COMMAND("port p2p", "set port adminPoit2Point")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	PARAM_ENUM("adminPoit2Point")
	PARAM_ENUM_SEL("y", "forcePointToPoint")
	PARAM_ENUM_SEL("n", "forcePointToMultiPoint")
	PARAM_ENUM_SEL("a", "autoPointToPoint")
	PARAM_ENUM_DEFAULT("a")
	THE_FUNC(stp_dcli_prt_p2p)

	THE_COMMAND("set ", "set vlan to instance")
	PARAM_NUMBER("vlan ", 1, STP_MAX_VID, "1")
	PARAM_NUMBER("instance number", 1, STP_MSTI_MAX, "0")
	THE_FUNC(stp_dcli_set_vlan_to_instance)

#ifdef STP_DBG
	THE_COMMAND("trace", "set port trace")
	PARAM_NUMBER("instance number", 1, STP_MSTI_MAX, "0")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	PARAM_ENUM("state machine name")
	PARAM_ENUM_SEL("receive", "receive")
	PARAM_ENUM_SEL("info", "info")
	PARAM_ENUM_SEL("roletrns", "roletrns")
	PARAM_ENUM_SEL("sttrans", "sttrans")
	PARAM_ENUM_SEL("topoch", "topoch")
	PARAM_ENUM_SEL("migrate", "migrate")
	PARAM_ENUM_SEL("transmit", "transmit")
	PARAM_ENUM_SEL("p2p", "p2p")
	PARAM_ENUM_SEL("edge", "edge")
	PARAM_ENUM_SEL("pcost", "pcost")
	PARAM_ENUM_DEFAULT("all")
	PARAM_BOOL("enable/disable", "trace it", "don't trace it", "n")
	THE_FUNC(stp_dcli_trace)

	THE_COMMAND("skip", "skip BPDU processing")
	PARAM_NUMBER("instance number of bridge", 0, STP_MSTI_MAX, "0")
	PARAM_NUMBER("port number", 1, NUMBER_OF_PORTS, "all")
	PARAM_ENUM("receive or/and transmit")
	PARAM_ENUM_SEL("rx", "receive")
	PARAM_ENUM_SEL("tx", "transmit")
	PARAM_ENUM_DEFAULT("all")
	PARAM_NUMBER("number of BPDU to skip", 0, 10000, "1")
	THE_FUNC(stp_dcli_skip)

	THE_COMMAND("sleep", "sleep")
	PARAM_NUMBER("delay in sec.", 1, 4000, "4")
	THE_FUNC(stp_dcli_sleep)
#endif
  
  END_OF_LANG
};

#ifdef STP_WITH_CLI
int stp_dcli_init (void)
{
   I_am_a_stupid_hub = 0;
   stp_cli_register_language (lang);
   return 0;
}
#endif
#ifdef __cplusplus
}
#endif

