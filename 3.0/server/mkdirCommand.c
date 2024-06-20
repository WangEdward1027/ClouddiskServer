#include "thread_pool.h"
#include <syslog.h>

//外部的数据库连接
//extern MYSQL * conn;

void mkdirCommand(task_t * task)
{
    User user={0};
    
    //获取新创建的文件名
    const char * mkDirName=task->data;
    
    char sendBuff[128];
    bzero(sendBuff,sizeof(sendBuff));
    
    char query[1024];
    bzero(query,sizeof(query));

    //数据库连接
    MYSQL *conn=create_db_connection();

    //获得userid
    //user.id=task->user->id;
    user.id = selectUserByUserName(task->user->userName)->id;

    if(user.id){
        sprintf(sendBuff,"获取 ‘userid’ 失败.\n");
        sendn(task->peerfd,sendBuff,sizeof(sendBuff));
        return;
    }
    else{
        sprintf(sendBuff,"获取的 ‘userid’ ：'%d'.\n",user.id);
        sendn(task->peerfd,sendBuff,sizeof(sendBuff));
        //此处还需要继续执行，不加return
    }

    //获得pwdid
    //user.pwd = selectUserByUserName(task->user->userName)->pwd;
    //
    sprintf(query,"SELECT pwdid FROM user WHERE userid=%d",user.id);
    int ret=mysql_query(conn,query);
    if(ret){
        //执行失败
        bzero(sendBuff,sizeof(sendBuff));
        sprintf(sendBuff,"%s\n",mysql_error(conn));
    }
    
    MYSQL_RES * result=mysql_use_result(conn);
    MYSQL_ROW rows=mysql_fetch_row(result);
    strcpy(user.pwd,rows[0]);
    mysql_free_result(result);
    
    bzero(sendBuff,sizeof(sendBuff));                
    sprintf(sendBuff,"当前工作目录  '%s'.\n",user.pwd);
    
    //获取parentid
    FileEntry* Flenry=(FileEntry*)calloc(1,sizeof(FileEntry)); 
    bzero(query,sizeof(query));
    sprintf(query, "SELECT parentid FROM fileentry WHERE owner_id = %d", user.id);
    
    ret = mysql_query(conn, query);
    result = mysql_store_result(conn);
    rows = mysql_fetch_row(result);
    Flenry->parentId = atoi(rows[0]);
    mysql_free_result(result);
    

    //查询目录是否已经存在
    bzero(query,sizeof(query));
    //测试写法，此处最好用联合查询的写法
    sprintf(query,"SELECT COUNT(filename) FROM fileentry WHERE parent_id=%d AND filename=%s AND owner_id=%d",
            Flenry->parentId,
            mkDirName,
            user.id);
    
    ret=mysql_query(conn,query);

    if(ret){                                        
        //执行失败                                 
        bzero(sendBuff,sizeof(sendBuff));          
        sprintf(sendBuff,"%s\n",mysql_error(conn));
    }                                              

    result=mysql_use_result(conn);
    rows=mysql_fetch_row(result);   
    strcpy(user.pwd,rows[0]);                 
    mysql_free_result(result);                

    //向虚拟文件表中插入数据
    bzero(query,sizeof(query));
    sprintf(query,"INSERT INTO fileentry(parent_id,filename,owner_id,md5,filesize,type) VALUES (%d,%s,%d,%s,%d,%d)",
            Flenry->parentId,mkDirName,user.id,0,0,0);
    mysql_query(conn,query);
    int affected_rows=mysql_affected_rows(conn);

    if(affected_rows==1){
        sprintf("创建目录  ’%s‘  成功.",mkDirName);
        sendn(task->peerfd,sendBuff,sizeof(sendBuff));
        syslog(LOG_INFO,"用户执行 ’MKDIR‘ 命令：成功.");
        
        mysql_close(conn);
        return;
    }
    else{
        sprintf("创建目录  '%s'  失败.",mkDirName);
        sendn(task->peerfd,sendBuff,sizeof(sendBuff));
        syslog(LOG_WARNING,"用户执行 ’MKDIR‘ 命令：失败.");
        
        mysql_close(conn);
        return;
    }
    
    mysql_close(conn);
    return;
}
