#include "thread_pool.h"

void lsCommand(task_t * task)
{
    int id; // 保存用户当前父目录id
    char sql[4096] = {0};
    char buff[1024] = {0};

    // 1. 判断ls后是否有参数
    if (strcmp(task->data, "") == 0) {
        // 当前ls后面无参数
        id = task->fileEntry->id;
    } else {
        // 1.1 构建sql语句
        sprintf(sql, "SELECT id FROM fileentry WHERE filename = %s", task->data);
        
        // 1.2 创立数据库连接
        MYSQL* mysql = create_db_connection();
        
        // 1.3 查询数据库
        int qret = mysql_query(mysql, sql);
        if (qret != 0) {
            // 查询失败，退出
            strcpy(buff, "输入的路径不存在!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;
        } 
        
        // 1.4 获取查询结果
        MYSQL_RES * res = mysql_store_result(mysql);
        if (res == NULL) {
            // 查询失败，退出
            strcpy(buff, "输入的路径不存在!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;
        }

        // 1.5 访问结果集
        MYSQL_ROW row;
        if ((row = mysql_fetch_row(res)) != NULL) {
            id = atoi(row[0]);
        }
        
        // 1.6 释放结果集
        mysql_free_result(res);

        // 1.7 关闭数据库的连接
        mysql_close(mysql);
    }
    
    // 2. 搜寻当前目录下的所有文件
    sprintf(sql, "SELECT filename FROM fileentry WHERE parent_id = %d", id);

    // 2.1 创建数据库连接
    MYSQL* mysql = create_db_connection();

    // 2.2 查询数据库
    int qret = mysql_query(mysql, sql);
    if (qret != 0) {
        // 查询失败
        strcpy(buff, "输入的路径不存在！\n");
        sendn(task->peerfd, buff, strlen(buff));
        return;
    }

    // 2.3

}

