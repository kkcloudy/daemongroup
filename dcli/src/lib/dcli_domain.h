#ifndef _DCLI_DRP_H_
#define _DCLI_DRP_H_


#define drp_err_show(ret) do{	\
	switch(ret){		\
		case DRP_ERR_DBUS_FAILED:		\
			vty_out(vty,"Dbus error !");		\
			break;			\
		case DRP_ERR_OPXML_FILE_NOT_EXIST:		\
			vty_out(vty,"domain xml config file not exist !");		\
			break;			\
		case	DRP_ERR_OPXML_FILE_FORMAT_WRONG:			\
			vty_out(vty,"domain xml file format wrong !");		\
			break;				\
		case	DRP_ERR_HANDLE_XML_DOMAIN_IP_EMPTY:			\
			vty_out(vty,"domain ip is empty !");				\
			break;							\
		case DRP_ERR_DNS_RESOLVE_PARAM_INPUT:			\
			vty_out(vty,"Unknown error !");				\
			break;				\
		case	DRP_ERR_DNS_RESOLVE_DOMAIN_EXIST:			\
			vty_out(vty,"domain already exist !");			\
			break;									\
		case	DRP_ERR_DNS_SERVER_NOT_CONFIGED:			\
			vty_out(vty,"dns server not configed !\n please config dns first!");		\
			break;									\
		case	DRP_ERR_DNS_RESOLVE_FAILED:			\
			vty_out(vty,"dns domain resolve failed !");				\
			break;				\
		case	DRP_ERR_DNS_RESULT_WRITE_XML_FAILED:				\
			vty_out(vty,"domain resolved write to config file failed !");		\
			break;				\
		case	DRP_ERR_DNS_DOMAIN_NOT_EXIST:		\
			vty_out(vty,"domain not exist !");			\
			break;		\	
		case	DRP_ERR_DNS_DOMAIN_IP_NOT_EXIST:		\
			vty_out(vty,"domain ip index not exist !");			\
			break;		\
		case	DRP_ERR_DNS_RESOLVE_IP_FORMAT_WRONG:		\
			vty_out(vty,"dns domain resolved ip address format wrong !");		\
			break;		\
		case	DRP_ERR_DNS_RESULT_XML_FORMAT_WRIONG:				\
			vty_out(vty,"domain config file format wrong !");		\
			break;							\
		case DRP_ERR_UNKNOWN:			\
			vty_out(vty,"Unknown error !");		\
			break;			\
		default:				\
			break;		\
		vty_out(vty,"Unknown error ! ret=%d",ret);		\
	}			\
}while (0)


#define _drp_return_if_fail(condition,error,ret_val) do {                      \
  if (!(condition)) {                                                          \
    drp_err_show(error);								 \
    return (ret_val);                                                                       \
 } } while (0)



void dcli_dnscache_init(void);

#endif
