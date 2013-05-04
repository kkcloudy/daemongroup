
#ifndef _AC_SAMPLE_MULTY_H__
#define _AC_SAMPLE_MULTY_H__

ac_sample_t *create_ac_sample_max_user(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );

ac_sample_t *create_ac_sample_radius_req(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );

ac_sample_t *create_ac_sample_ip_pool(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );

ac_sample_t *create_ac_sample_porter_req(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );

ac_sample_t *create_ac_sample_dhcp_exhaust(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );

ac_sample_t *create_ac_sample_find_attack(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );

enum opt_rtn_en
{
	OPT_RTN_OK,
	OPT_RTN_FILE_NOT_EXIST,
	OPT_RTN_FILE_ERR,
};

struct instance_radius_req {
    unsigned int local_id;
    unsigned int instance_id;

    unsigned int pre_instance_status;
    unsigned int instance_status;
    
	struct list_head node;

    unsigned long access_request_count;
    unsigned long access_accept_count;
    unsigned long access_accept_rate;

    unsigned int last_over_time;
    unsigned int last_status;
};


#endif


