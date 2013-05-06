#ifndef _FILE_TRANSE_H
#define _FILE_TRANSE_H

typedef enum{
	LOAD_FLAG_BEGIN=0,
	DOWNLOAD,
	UPLOAD,
	LOAD_FLAG_END
}LOAD_FLAG;

typedef enum{
	TRANS_FLAG_FOREGROUND=1,		/*传输命令在前台执行*/
	TRANS_FLAG_BACKGROUND
}TRANS_FLAG;


typedef enum{
	PROTOCAL_BEGIN=0,
	PROTOCAL_TFTP,
	PROTOCAL_FTP,				/*传输协议上，优先做这个，其它协议暂时不考虑。*/
	PROTOCAL_SFTP,
	PROTOCAL_HTTP,
	PROTOCAL_HTTPS,
	PROTOCAL_END
}TRANS_PROTOCAL;


typedef enum{
	TRANS_STATUS_BEGIN=0,
	TRANSFERRING,
	COMPLETED,
	FAILED,
	TRANS_STATUS_END
}TRANS_STATUS;

typedef struct file_trans_t file_trans_t;
typedef int (*ON_TRANS_FINISH)( struct file_trans_t *me, void *param );

file_trans_t *file_trans_create();
int file_trans_destroy( file_trans_t *me );

int ft_set_loadflag( file_trans_t *me, LOAD_FLAG flag);
int ft_set_protocal( file_trans_t *me, TRANS_PROTOCAL portal);
int ft_set_ipaddr  ( file_trans_t *me, char *ipaddr );
int ft_set_port    ( file_trans_t *me, unsigned int port );
int ft_set_srvpath( file_trans_t *me, char *srvpath);
int ft_set_userpath(file_trans_t *me, char *userpath);
int ft_set_username( file_trans_t *me, char *username );
int ft_set_password( file_trans_t *me, char *password );
int ft_set_onfinish( file_trans_t *me, ON_TRANS_FINISH on_finish );

#define FT_OK						0
#define FT_ERR					(FT_OK-1)
#define FT_MALLOC_ERR		(FT_OK-2)
#define FT_POINTER_NULL	(FT_OK-3)

#define MAX_LEN 256

/*
*这几个get意义不大，可以先不实现。
LOAD_FLAG 			ft_get_loadflag ( file_trans_t *me );
char 						*ft_set_filename( file_trans_t *me );
TRANS_PROTOCAL 	ft_set_protocal ( file_trans_t *me );
unsigned int 		ft_set_ipaddr   ( file_trans_t *me );
int 						ft_get_port     ( file_trans_t *me );
char 						*ft_get_username( file_trans_t *me );
char 						*ft_set_password( file_trans_t *me );
*/

TRANS_STATUS 		ft_get_status		( file_trans_t *me );
char 				*ft_get_failedreason( file_trans_t *me );



/*the pivotal function*/
/*
*	执行文件传输动作！
*trans_flag = TRANS_FLAG_FOREGROUND: 当文件传输结束后才返回
*trans_flag = TRANS_FLAG_BACKGROUND: 函数立即返回，文件在后台传输。
TRANS_FLAG_BACKGROUND这个模式下，在传输的过程中可以调用下面的函数来获取一些传输的状态
int 	  ft_get_curent_trans_size( file_trans_t *me );->获得文件以传输的部分的大小。
TRANS_STATUS 		ft_get_status		( file_trans_t *me );->得到当前的传输状态，为TRANS_STATUS　之一
**/
int		ft_do_transfers( file_trans_t *me);


/****************************************************
*	当执行的　trans_flag = TRANS_FLAG_BACKGROUND，并且LOAD_FLAG为DOWNLOAD的时候
*通过这个函数得到当前文件已经下载了多少字节。
*
*对于LOAD_FLAG为UPLOAD的情况，要得到已经传输了多少字节是乎比较困难。可以考虑一下。
*****************************************************/
int 	  ft_get_curent_trans_size( file_trans_t *me );




#endif
