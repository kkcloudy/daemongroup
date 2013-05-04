#ifndef _IPT_CONNLIMIT_H
#define _IPT_CONNLIMIT_H

struct ipt_connlimit_data;

struct ipt_connlimit_info {
	int limit;
	int inverse;
	u_int32_t mask;
	struct ipt_connlimit_data *data __attribute__((aligned(8)));
};
#endif /* _IPT_CONNLIMIT_H */
