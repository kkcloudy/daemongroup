#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ioctl.h>
#include <errno.h>


struct clear_radio_if {
	unsigned int vrid;
};

#define WIFI_IOC_MAGIC		243
#define WIFI_IOC_RADIO_IF_CLEAR	_IOWR(WIFI_IOC_MAGIC, 17, struct clear_radio_if)

#define DEV_PATH	"/dev/wifi0"


int main(int argc, char **argv)
{
	int ret ;
	int fd;
	struct clear_radio_if temp_t;
	unsigned long long_vrid;	
	if (argc < 2) {
		printf("command need assign vrrpid\n");
		//return 1;
		exit(1);
	}
	
	long_vrid = strtol(argv[1], NULL, 10);
	if (errno != 0) {
		printf("error vrid: %s errno=%d\n", argv[1], errno);
		//return 2;
		exit(2);
	}

	temp_t.vrid = (unsigned int )long_vrid;

	if (temp_t.vrid >= 35) {
		printf("vrrpid %d is out of range [0, 35)\n");
		//return 3;
		exit(3);
	} else {
		printf("ready to clear vrid %d all radio interfaces\n", temp_t.vrid);
	}

	fd = open(DEV_PATH, 0);
	if (fd < 0) {
		printf("%s open failed\n", DEV_PATH);
		//return 4;	
		exit(4);
	}	
	
	ret = ioctl(fd, WIFI_IOC_RADIO_IF_CLEAR, &temp_t);

	if (ret < 0 ) {
		printf("clear wifi radio interface failed ret = %d errno=%d\n", ret, errno);
		close(fd);
		//return 5;
		exit(5);
	} else {
		printf("clear vrid %d radio interfaces done\n", temp_t.vrid);
		close(fd);
		//return 0;
		exit(0);
	}
}

