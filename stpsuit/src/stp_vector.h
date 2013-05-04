/* STP priority vectors API : 17.4.2 */
 
#ifndef _STP_PRIO_VECTOR_H__
#define _STP_PRIO_VECTOR_H__

typedef struct bridge_id
{
  unsigned short    prio;
  unsigned char     addr[6];
} BRIDGE_ID;

typedef unsigned short  PORT_ID;

typedef struct prio_vector_t {
  BRIDGE_ID root_bridge;
  unsigned long root_path_cost;
  BRIDGE_ID region_root_bridge;
  unsigned long region_root_path_cost;
  BRIDGE_ID design_bridge;
  PORT_ID   design_port;
  PORT_ID   bridge_port;
} PRIO_VECTOR_T;

void 
stp_vector_create (OUT PRIO_VECTOR_T* t,
                 IN BRIDGE_ID* root_br,
                 IN unsigned long root_path_cost,
                 IN BRIDGE_ID* region_root_br,
                 IN unsigned long region_root_path_cost,
                 IN BRIDGE_ID* design_bridge,
                 IN PORT_ID design_port,
                 IN PORT_ID bridge_port);
void
stp_vector_copy (OUT PRIO_VECTOR_T* t, IN PRIO_VECTOR_T* f);

int
stp_vector_compare_bridge_id (IN BRIDGE_ID* b1, IN BRIDGE_ID* b2);

int
stp_vector_compare (IN PRIO_VECTOR_T* v1, IN PRIO_VECTOR_T* v2);

void
stp_vector_get_vector (IN BPDU_BODY_T* b, OUT PRIO_VECTOR_T* v);

void
stp_vector_set_bridge_id (IN BRIDGE_ID* bridge_id, OUT unsigned char* c_br);

void
stp_vector_set_vector (IN PRIO_VECTOR_T* v, OUT BPDU_BODY_T* b);

#ifdef STP_DBG
void
stp_vector_print (IN char* title, IN PRIO_VECTOR_T* v);

void
stp_vector_br_id_print (IN char *title, IN BRIDGE_ID* br_id, IN Bool cr);

#endif

#endif /* _STP_PRIO_VECTOR_H__ */


