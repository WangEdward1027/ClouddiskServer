#include "thread_pool.h"

char* getParentPath(const char* path) {
    const char* delimiter = "/";
    char* token;
    char* lastToken = NULL;
    char* secondLastToken = NULL;
    char* result = NULL;

    // 使用 strtok 函数逐个切割路径
    token = strtok((char*)path, delimiter);
    while (token != NULL) {
        secondLastToken = lastToken;
        lastToken = token;
        token = strtok(NULL, delimiter);
    }

    if (secondLastToken != NULL) {
        // 获取上层目录的路径长度
        size_t parentDirLength = secondLastToken - path;

        // 创建新的字符串来存储上层目录的路径
        result = (char*)malloc(parentDirLength + 1);
        strncpy(result, path, parentDirLength);
        result[parentDirLength] = '\0';
    }

    return result;
}

void removeTrailingSpace(char* str) {
    int length = strlen(str);
    if (length > 0 && str[length - 1] == ' ') {
        str[length - 1] = '\0';
    }
}

FileEntry* selectFileEntryByFileNameParentIdOwnerId(const char* fileName, int parentId, int ownerId) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM fileentry WHERE filename = '%s' AND parent_id = %d AND owner_id = %d", fileName, parentId, ownerId);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectFileEntryByFileNameParentIdOwnerId() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_ROW row;
    FileEntry *fileEntry = NULL;
    if ((row = mysql_fetch_row(res))) {
        fileEntry = (FileEntry *)malloc(sizeof(FileEntry));
        fileEntry->id = atoi(row[0]);
        fileEntry->parentId = atoi(row[1]);
        strcpy(fileEntry->fileName, row[2]);
        fileEntry->ownerId = atoi(row[3]);
        strcpy(fileEntry->md5, row[4]);
        fileEntry->fileSize = atoi(row[5]);
        fileEntry->fileType = atoi(row[6]);
    }

    mysql_free_result(res);
    mysql_close(conn);
    return fileEntry;
}

char* removeLastPathComponent(const char* path) {
    if (path == NULL || strlen(path) == 0)
        return NULL;

    char* copy = strdup(path); // 复制输入路径以避免修改原始字符串
    char* lastSlash = strrchr(copy, '/'); // 查找最后一个斜杠字符

    if (lastSlash == NULL) {
        free(copy);
        return NULL;
    }

    *lastSlash = '\0'; // 将最后一个斜杠替换为字符串结束符

    char* parentPath = strdup(copy); // 复制前面的内容
    free(copy); // 释放复制的字符串内存

    return parentPath;
}


void cdCommand(task_t* task) {
    
    char buff[512] = {0};
    char* filename;
    char path[128] = {0};
    
    // 根据username查表获取user表信息，再通过user表信息获取pwd
    char * username = task->user->userName;
    User * user = selectUserByUserName(username);
    char * pwd = user->pwd;
    // printf("初始进入cd命令时，此时的pwd为%s\n", pwd);

    // 移除指令后的空格
    removeTrailingSpace(task->data);

    // 1. 判断cd 参数是否合法
    if (strcmp(task->data, "") == 0) {
        strcpy(buff, "请输入正确的cd 命令!\n");
        strcat(buff, "\0");
        sendn(task->peerfd, buff, strlen(buff));
        return;

    } else if (strcmp(task->data, ".") == 0 || strcmp(task->data, "./") == 0) {
        sprintf(buff, "当前路径为>%s", pwd);
        sendn(task->peerfd, buff, strlen(buff));
        return;

    } else if (strcmp(task->data, "../") == 0 || strcmp(task->data, "..") == 0) {
        printf("进入../\n");
       //  printf("1.此时pwd的值为：%s\n",pwd);
        // 1. 先获取当前目录的虚拟文件表信息
        char tmp[64] = {0};
        strcpy(tmp, pwd); // 用tmp暂存当前的pwd,防止被更改
        FileEntry * curr = selectFileEntryByFileName(getCurrentDirectory(tmp));
        // 1.1 判断当前的目录虚拟文件表的parent_id是否是0
        if (curr->parentId == 0) {
            // 当前已经是根目录，返回提示
            sprintf(buff, "当前已在根目录!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;
        }
        // 1.2 当前目录虚拟文件表的parent_id不是0，代表还有上层目录
        // 获取要切换的目录
        // printf("获取父目录前的pwd为:%s\n", pwd);
        char * res = removeLastPathComponent(pwd);
        // printf("获取父目录的结果为:%s\n", res);
        strcpy(path, res);
        printf("当前的目录为%s\n", path);
        sprintf(buff, "当前路径为>%s\n", path);
        // 发送给客户端
        sendn(task->peerfd, buff, strlen(buff));
        // 更新user表的pwd信息
        strcpy(user->pwd, path);
        updateUser(user);
        return;

    } else {
        
        // cd 到 mc,mc一定在当前目录下，需要查找当前目录下是否存在对应的文件名
        // 获取当前目录的虚拟文件表
        char temp[64] = {0};
        strcpy(temp,pwd);
        FileEntry * curr = selectFileEntryByFileName(getCurrentDirectory(temp));

        // 获取要切换的目录名
        filename = getCurrentDirectory(task->data);
        // 判断切换的目录是否存在
        FileEntry* dir = selectFileEntryByFileNameParentIdOwnerId(filename, curr->id, curr->ownerId);
        
        if (dir == NULL) {
            sprintf(buff, "要切换的目录不存在!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;
        }
        
        bzero(path,sizeof(path));
        strcat(path, pwd); // 添加当前pwd
        strcat(path, "/");
        strcat(path, task->data);
        sprintf(buff, "当前路径为>%s", path);
        sendn(task->peerfd, buff, strlen(buff));

        // 更新当前path
        strcpy(user->pwd, path);
        updateUser(user); // 将更改发送到数据库
        
        return;
    }
}
