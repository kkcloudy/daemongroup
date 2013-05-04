#ifndef DCLI_LOCAL_HANSI
#define DCLI_LOCAL_HANSI
#define SHOW_RUNNING_CONFIG_LEN	(1024 * 1024)
int dcli_license_show_running_config(struct vty *vty);
int dcli_hmd_hansi_is_running
(
	struct vty* vty,
	unsigned int slot_id,
	int islocal,
	int instID
);
#endif
