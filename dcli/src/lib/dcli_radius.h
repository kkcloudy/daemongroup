#ifndef DCLI_RADIUS_H
#define DCLI_RADIUS_H

#define DEF_RULER_NUM  8

struct  sql_conf
    {
            char serverip[50];
	     char login[128];
	     char mysqlpassword[128];
    };

#define MYSQL_OK                                  0
#define MYSQL_ERR_BASE                     0
#define MYSQL_ERR                                (MYSQL_ERR_BASE-1)
#define MYSQL_ERR_MALLOC_ERR        (MYSQL_ERR_BASE-2)
#define MYSQL_ERR_PORT_ERR             (MYSQL_ERR_BASE-3) 
#define MYSQL_ERR_FILEOPEN_ERR      (MYSQL_ERR_BASE-4) 

#endif

