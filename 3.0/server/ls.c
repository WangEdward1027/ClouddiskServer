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
    char buff[2048] = {0};
    char* filename;

    // 1. 判断ls后是否有参数
    if (strcmp(task->data, "") == 0) {
        // 当前ls后面无参数
        filename = getCurrentDirectory(task->user->pwd);
    } else if (strcmp(task->data, "./") == 0 || strcmp(task->data, ".") == 0){
        filename = getCurrentDirectory(task->user->pwd); 
    } else if (strcmp(task->user->pwd, "../") == 0 || strcmp(task->data, "..") == 0) {
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

