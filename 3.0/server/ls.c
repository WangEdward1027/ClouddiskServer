#include "thread_pool.h"

char* getCurrentDirectory(const char* pwd) {
    const char* delimiter = "/";
    char* token;
    char* lastToken = NULL;

    // 使用 strtok 函数逐个切割路径
    token = strtok((char*)pwd, delimiter);
    while (token != NULL) {
        lastToken = token;
        token = strtok(NULL, delimiter);
    }

    if (lastToken != NULL) {
        // 创建一个新的字符串存储目录名
        char* result = strdup(lastToken);
        return result;
    }

    return NULL;
}

char* getParentDirectory(const char* pwd) {
    const char* delimiter = "/";
    char* token;
    char* lastToken = NULL;
    char* secondLastToken = NULL;

    // 使用 strtok 函数逐个切割路径
    token = strtok((char*)pwd, delimiter);
    while (token != NULL) {
        secondLastToken = lastToken;
        lastToken = token;
        token = strtok(NULL, delimiter);
    }

    if (secondLastToken != NULL) {
        // 创建一个新的字符串存储上级目录
        char* result = strdup(secondLastToken);
        return result;
    }

    return NULL;
}


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
    int parent_id; // 保存用户当前父目录id
    char buff[2048] = {0};
    char* filename;

    // 1. 判断ls后是否有参数
    if (strcmp(task->data, "") == 0) {
        // 当前ls后面无参数
        filename = getCurrentDirectory(task->user->pwd);
    } else if (strcmp(task->data, "./") == 0){
        filename = getCurrentDirectory(task->user->pwd); 
    } else if (strcmp(task->user->pwd, "../") == 0) {
        filename = getParentDirectory(task->user->pwd);
        if (filename == NULL) {
            strcpy(buff, "无效命令，重新输入。\n");
            strcat(buff, "\0");
            sendn(task->peerfd, buff, strlen(buff));
            return;
        }
    } else {
        filename = getCurrentDirectory(task->data);
    }

    // 2. 获取filename 对应的id
    FileEntry* dir = selectFileEntryByFileName(filename);
    if (dir == NULL) {
        strcpy(buff, "无效命令，重新输入。\n");
        strcat(buff, "\0");
        sendn(task->peerfd, buff, strlen(buff));
        return;
    }
    
    parent_id = dir->id; // 保存对应id
    
    free(dir);

    // 3. 根据parent_id 查询对应的记录
    FileEntry* reslist = selectFileEntryByparentId(parent_id);
    
    int i = 0;
    // 4. 输出对应结果
    while (reslist != NULL) {
        sprintf(filename, "%-15s\t", reslist->fileName);
        strcat(buff, filename);
        i++;
        if (i % 4 == 0) {
            strcat(buff, "\n");
        }
        reslist++;
    }
    strcat(buff, "\0");
    
    sendn(task->peerfd, buff, strlen(buff));
    
    return;
}

