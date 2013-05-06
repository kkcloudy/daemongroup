#ifndef _NPD_IF_H_
#define _NPD_IF_H_

#include <net/if.h>

#define IN
#define OUT

inline int if_name2gindex
(
	IN char *name,
	OUT int *gindex
) 
{
#define SIOCGGINDEX	0x8939	/* get global index of the device */
	struct ifreq	ifr;
	int fd = -1;
	//int iocmd = 0;

	if(!name){
		return (-1);
	}
	memset(&ifr, 0, sizeof(struct ifreq));

	/* init socket */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		return (-2);
	}

	/* build up ioctl arguments */
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	/* do I/O */
	if (ioctl(fd, SIOCGGINDEX, (char *)&ifr)){
		return -errno;
	}

	close(fd);
	*gindex = ifr.ifr_ifindex;

	return 0; 
} 

#endif
