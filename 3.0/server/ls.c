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



void lsCommand(task_t * task)
{
    int parent_id; // 保存用户当前父目录id
    char buff[1024] = {0};
    char* filename;

    // 根据当前的username 找到对应的use表信息从而得到对应的pwd,保存到path里
    char * username = task->user->userName;
    User * user = selectUserByUserName(username);
    char * path = user->pwd;

    // 1. 判断ls后是否有参数
    if (strcmp(task->data, "") == 0) {
        // 当前ls后面无参数
        filename = getCurrentDirectory(path);
    } else if (strcmp(task->data, "./") == 0 || strcmp(task->data, ".") == 0){
        filename = getCurrentDirectory(path); 
    } else if (strcmp(task->data, "../") == 0 || strcmp(task->data, "..") == 0) {
        filename = getParentDirectory(path);
        if (filename == NULL) {
            strcpy(buff, "当前已在根目录，重新输入。\n");
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
        strcpy(buff, "数据库找不到对应的目录，重新输入。\n");
        strcat(buff, "\0");
        sendn(task->peerfd, buff, strlen(buff));
        return;
    }
    
    parent_id = dir->id; // 保存对应id
    
    free(dir);
    
    int num = 0;
    // 3. 根据parent_id 查询对应的记录
    FileEntry* reslist = selectFileEntryByparentId(parent_id, &num);
    
    // 4. 写入ls数据
    if (reslist == NULL) {
        sprintf(buff, " ");     
    }
    else {
        for (int i = 1; i <= num; i++) {
    if (reslist[i-1].fileType == 0) { // 目录
        sprintf(filename, "\033[0;34m%-12s\033[0m\t", reslist[i-1].fileName); // 蓝色
    } else if (reslist[i-1].fileType == 1) { // 文件
        sprintf(filename, "\033[0;32m%-12s\033[0m\t", reslist[i-1].fileName); // 绿色
    }
    strcat(buff, filename);
    if (i % 5 == 0) {
        strcat(buff, "\n");
    }
        }
        strcat(buff, "\0");
    }
    sendn(task->peerfd, buff, strlen(buff));
    
    return;
}

