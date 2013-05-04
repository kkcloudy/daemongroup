#ifndef _MIBIF_H
#define _MIBIF_H

#define MIBIF_DEBUG		0

#if MIBIF_DEBUG
#define MIBIF_PRINTF	printf("%s---%d:",__FILE__, __LINE__);printf
#else
#define MIBIF_PRINTF(...)
#endif
	

#define MAX_MIBIF_IFNAME_LEN		32
#define MAX_MIBIF_SUBIF_INFO_LEN	256	



typedef struct mibif_t {
	char ifname[MAX_MIBIF_IFNAME_LEN];
	int  gindex;
	int	 index;
	unsigned int ipaddr;
	unsigned int mask;
} mibif;

typedef struct mibif_lst_t{
	int mibif_num;
	int buf_mibif_num;
	mibif *lst;
} mibiflst;

typedef struct mibsubif_t {
	char ifname[MAX_MIBIF_IFNAME_LEN];
	int  gindex;
	int	 index;/*read only*/
	unsigned int ipaddr;
	unsigned int mask;
	int  subif_vlanid;/*for sub if*/
	char main_ifname[MAX_MIBIF_IFNAME_LEN];
	char info[MAX_MIBIF_SUBIF_INFO_LEN];/*for sub if*/
} mibsubif;

typedef struct mibsubif_lst_t{
	int mibif_num;
	int buf_mibif_num;
	mibsubif *lst;
} mibsubiflst;

/*0、得到当前所有三层接口的列表*/
int get_all_mibif( mibiflst *iflst );/*使用完成后需要释放!!!!  返回值为if个数*/
int get_all_submibif( mibsubiflst *subiflst );/*使用完成后需要释放!!!!  返回值为if个数*/

/*1、得到当前接口是否为主控口*/
int if_main_mibif( const char *name );
/*只有这些接口下才能创建子接口，当前的针对性很强，只能正对7000的0-1~0-4,当前5612有主控口，但是还不能创建子接口,其实质就是在上面函数中进行ifname的过滤*/


/*2、得到当前接口是否为子接口*/
int if_sub_mibif( const char *name );
/*同上，也是根据ifname得来的，当前只能针对7000，其它设备都得不到*/


/*3、创建子接口，针对子接口的描述文字，需要有文件来保存。使用最简单的文本文件。*/
int create_sub_mibif( int vlanid, char *main_if, char *info );
	
	
/*4、删除子接口，*/
int delete_sub_mibif( int vlanid, char *main_if );
	
	
/*5、修改三层接口的ip地址*/
int set_mibif_ip( mibif *mif, unsigned int ipaddr );


/*6、修改三层接口的mask*/
int set_mibif_mask( mibif *mif, unsigned int mask );

#define SIOCGGINDEX		0x8939		/* get global index of the device */
#define SIOCSGINDEX 	0x893A		/* set global index of the device */
/*修改三层接口的gindex*/
int set_mibif_gindex( mibif *mif, unsigned int gindex );

int mod_sub_mibif_mainif( int vlanid, char *old_main_if, char *new_main_if );
int mod_sub_mibif_gindex( int vlanid, char *main_if, int gindex );

#endif
