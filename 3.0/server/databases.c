#include"thread_pool.h"


#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS 1234
#define DB_NAME "58th"

//创建数据库连接
MYSQL* create_db_connection(){
    MYSQL*conn=mysql_init(NULL);
    if(conn==NULL){
        fprintf(stderr,"mysql_init() failed\n");
        return NULL;
    }

    if(mysql_real_connect(conn,DB_HOST,DB_USER,DB_PASS,DB_NAME,0,NULL,0)==NULL){
        fprintf(stderr,"mysql_real_connect() failed\n");
        mysql_close(conn);
        return NULL;
    }
    return conn;
}
