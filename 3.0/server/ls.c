#include "thread_pool.h"

bool do_select(task_t* task, char* sql) {
    
    char buff[1024] = {0};
    char filename[20] = {0};
    // 1. 创建数据库连接
    MYSQL* mysql = create_db_connection();

    // 2. 查询数据库
    int qret = mysql_query(mysql, sql);
    if (qret != 0) {
        // 查询失败，退出
        strcpy(buff, "输入无效的路径!!!\n");
        sendn(task->peerfd, buff, strlen(buff));
        return false;
    }

    // 3. 获取查询结果
    MYSQL_RES* res = mysql_store_result(mysql);
    if (res == NULL) {
        // 查询失败，退出
        strcpy(buff, "获取结果出错!!!\n");
        sendn(task->peerfd, buff, strlen(buff));
        return false;
    }

    // 4. 访问结果集
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) {
        sprintf(filename, "%-15s\t", row[0]);
        strcat(buff, filename);
    }
    
    // 5. 返回查询结果
    sendn(task->peerfd, buff, strlen(buff));

    // 6. 释放结果集
    mysql_free_result(res);

    // 关闭数据库连接
    mysql_close(mysql);

    return true;
}


void lsCommand(task_t * task)
{
    int id; // 保存用户当前父目录id
    char sql[4096] = {0};

    // 1. 判断ls后是否有参数
    if (strcmp(task->data, "") == 0) {
        // 当前ls后面无参数
        id = task->fileEntry->id;
        sprintf(sql, "SELECT filename FROM fileentry WHERE parent_id = %d", id);
    } else {
        // 当前ls后面有参数
        sprintf(sql, "SELECT filename FROM fileentry WHERE parent_id = (SELECT id FROM fileentry WHERE filename = %s)", task->data);
    }
    
    // 2. 执行sql
    do_select(task, sql);
    
    return;
}

