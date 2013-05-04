#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <net/if.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_log.h"
#include "pppoe_util.h"


long
time_sysup(void) {
	struct sysinfo info;
	sysinfo(&info);	
	return info.uptime;
}

uint32 
get_product_info(char *filename) {
	char buf[32];
	uint32 data;
	int fd;
	
	if (!filename)
		goto error;

    fd = open(filename, O_RDONLY, 0);
	if (fd < 0) 
		goto error;

	if (read(fd, buf, 16) < 0)
		goto error1;

	data = strtoul(buf, NULL, 10);

	close(fd);
	return data;

error1:
	close(fd);
error:
	return 0;
}

uint32 
ifname_get_slot_id(const char *ifname) {
	uint32 slotnum = 0;
	
	if (0 == strncmp(ifname, "eth", 3)) {			/* eth : cpu */
		sscanf(ifname, "eth%u-%*u", &slotnum);
	} else if (0 == strncmp(ifname, "ve", 2)) {		/* ve */
		sscanf(ifname, "ve%uf%*u.%*u", &slotnum);
	} else if (0 == strncmp(ifname, "r", 1)) {		/* radio */
		sscanf(ifname, "r%u-%*u-%*u-%*u.%*u", &slotnum);
	} else if (0 == strncmp(ifname, "wlan", 4)) {	/* wlan */
		sscanf(ifname, "wlan%u-%*u-%*u", &slotnum);
	} else if (0 == strncmp(ifname, "ebr", 3)) {	/* ebr */
		sscanf(ifname, "ebr%u-%*u-%*u", &slotnum);
	}
	
	return slotnum;
}

int 
netmask_check(uint32 mask) {
	int i;
	
	if (mask < (1 << 31))
		return PPPOEERR_EINVAL;

	for (i = 30; i >= 0; i--) {
		if ((mask & (1<<i)) && !(mask & (1 << (i + 1))))
			return PPPOEERR_EINVAL;
	}
	
	return PPPOEERR_SUCCESS;
}

int
set_nonblocking(int fd) {
	char errbuf[128];
	int flags;

	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		pppoe_log(LOG_WARNING, "fcntl(F_GETFL) failed for fd %d: %s\n",
							fd, strerror_r(errno, errbuf, sizeof(errbuf)));
		return PPPOEERR_ESYSCALL;
	}
	
	if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0) {
		pppoe_log(LOG_WARNING, "fcntl failed setting fd %d non-blocking: %s\n",
							fd, strerror_r(errno, errbuf, sizeof(errbuf)));
		return PPPOEERR_ESYSCALL;
	}
	
	return PPPOEERR_SUCCESS;;
}

int 
ifname_detect_exist(const char *ifname) {
	struct ifreq ifr;
	int sk, ret;

	if (!ifname || !ifname[0])
		return PPPOEERR_EINVAL;
	
	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk < 0)
		return PPPOEERR_ESOCKET;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	ret = ioctl(sk, SIOCGIFINDEX, &ifr);
	
	close(sk);
	return ret ? PPPOEERR_ENOEXIST : PPPOEERR_SUCCESS;
}

void
rand_seed_init(uint8 *seed, uint32 size) {
	struct timeval tv;
	uint32 pid = getpid(), usec;
	int i = 0;

	gettimeofday(&tv, NULL);
	usec = tv.tv_usec;
	
	for (; i < size; i++) {
		if (0 == i || 1 == i) {
			seed[i] = (pid >> (i * 8)) & 0xff;
		} else {
			seed[i] = rand_r(&usec) & 0xff;
		}
	}
}

void
rand_bytes(uint32 *seed, uint8 *buf, uint32 size) {
	int i = 0;
	for (; i < size; ++i)
		buf[i] = rand_r(seed) & 0xff;
}

