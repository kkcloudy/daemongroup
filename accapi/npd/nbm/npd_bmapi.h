#ifndef __NPD_BMAPI_H__
#define __NPD_BMAPI_H__

#define	BIT_IS_SET(data,bit)	((1<<bit) & data)
#define BIT_IS_CLEAR(data,bit) (!(BIT_IS_SET(data,bit)))
#define BIT_SET(data,bit)		(data |= (1<<bit))
#define BIT_CLEAR(data,bit)	(data &= ~(1<<bit))

#define AX7_PRODUCT_SLOT_NUM	5
#define AX7_CRSMU_SLOT_NUM	0

#define AX7_FATAL_ERR_MSG_PRINT_INTERVAL 300   /* every  five minute interval print out once if fatal error(e.g. excessive temperature) occurred */

#define AX7i_SLAVE_SLOT_NUM 2
#define AX7i_XG_CONNECT_SLOT_NUM 3

/*
  NOTICE
  This file define data structures that are shared between npd and nbm
*/

typedef struct{
	unsigned char fan_porwer;
	unsigned short inter;
	unsigned short surface;
}NPD_ENVI;

struct module_type_data_s {
	unsigned char ext_slot_count;
	/*
	Usually extend slot count would be 0, could be 1 or 2 in some cases,
	especially in box product with extend slots.
	*/
	unsigned char eth_port_start_no;
	/* First port no printed in the panel, Usually it should be 0 or 1;
	It might be some special value if the board have some special other port.
	*/
	unsigned short eth_port_count;
    /*
	Data field for other kinds of physical port and extend slots
	could be added here when needed
	*/
};

struct module_info_s {
	enum module_id_e id;
	enum module_status_e state;
	unsigned char hw_version;
	char *modname;
	char *sn;
	void *ext_slot_data;   /* extend info could be organized as a link list.*/
};


struct chassis_slot_s {
	struct module_info_s  module;
};

struct product_type_data_s {
	unsigned char chassis_slot_count;    /* 1 for box product.*/
	unsigned char chassis_slot_start_no;   /* first no, usually it should be 0, sometimes it could be other value*/
	unsigned char chassis_subcard_count;   /* 4626 has 2 subcards, 5612 has 1 subcard, 5612i has 1 subcard*/
	unsigned short padding;
};

struct product_sysinfo_s {
	char *sn;
	char *name;
	char *basemac;  /* 12 char of mac address  with no : or - spliter.*/
	char *sw_name;
	char *enterprise_name;
	char *enterprise_snmp_oid;
	char *snmp_sys_oid;
	char *built_in_admin_username;
	char *built_in_admin_passwd;
};

struct product_s {
	enum module_id_e local_module_id;
	unsigned char local_module_hw_version;
	enum product_id_e product_id;
	int local_chassis_slot_no;  /* number printed in the front panel of the module on which this software runs*/
	char *chassis_backplane_sn;
	char *chassis_backplane_name;
	struct product_sysinfo_s sysinfo;
	unsigned int capbmp;
	unsigned int runbmp;
};

/* all system accessories running state collection*/
struct system_temperature_s {
	unsigned short         current; /* realtime temperature*/
	unsigned short         criticalH; /* warning baseline - temperature greater than this will give warning message*/
	unsigned short         criticalL; /* cancel warning when temperature less than this*/
	unsigned char	       flag;
	
};

struct system_state_s {
	enum system_fan_status_e fan_state;
	unsigned char	         power_state;
	struct system_temperature_s core; /* CPU core temperature*/
	struct system_temperature_s surface; /* CPU surface temperature*/
};


/* if value in range [min,max]*/
#define VALUE_IN_RANGE(val,min,max) (((val) >= (min)) && ((val) <= (max)))

#endif
