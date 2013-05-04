#ifndef _AC_SAMPLE_CONTAINER_H
#define _AC_SAMPLE_CONTAINER_H

#include "ac_sample.h"

#if 0
#define RTN_ASC_OK          0
#define RTN_ASC_ERR         (RTN_ASC_OK-1)
#define RTN_ASC_NOT_INIT    (RTN_ASC_OK-2)
#define RTN_ASC_ENTRY_ARREADY_IN_LIST   (RTN_ASC_OK-3)
#define RTN_ASC_ENTRY_NOT_IN_LIST       (RTN_ASC_OK-4)
#endif

int init_sample_container();
int uninit_sample_container();

int register_ac_sample( ac_sample_t *pas );
int unregister_ac_sample( ac_sample_t *pas );

ac_sample_t *get_ac_sample_by_name( char *name );

int ac_sample_dispach();


int set_all_sample_interval( unsigned int sample_interval );
unsigned int get_all_sample_interval();


int set_all_statistics_time( unsigned int statistics_time );
unsigned int get_all_statistics_time( );


int set_ac_sample_service_state(int state);

int get_ac_sample_service_state( );



#endif

