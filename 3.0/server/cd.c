#include "thread_pool.h"

#define MAX_SIZE 100

typedef struct {
    char** strings;  // 字符串数组
    int top;         // 栈顶指针
} StringStack;

StringStack* createStringStack() {
    StringStack* stack = (StringStack*)malloc(sizeof(StringStack));
    stack->strings = (char**)malloc(MAX_SIZE * sizeof(char*));
    stack->top = -1;
    return stack;
}

void destroyStringStack(StringStack* stack) {
    if (stack == NULL) {
        return;
    }

    // 释放栈中的字符串内存
    for (int i = 0; i <= stack->top; i++) {
        free(stack->strings[i]);
    }

    free(stack->strings);
    free(stack);
}

int isStringStackEmpty(StringStack* stack) {
    return stack->top == -1;
}

int isStringStackFull(StringStack* stack) {
    return stack->top == MAX_SIZE - 1;
}

void pushString(StringStack* stack, const char* str) {
    if (isStringStackFull(stack)) {
        printf("Error: String stack is full.\n");
        return;
    }

    int length = strlen(str);
    char* newString = (char*)malloc((length + 1) * sizeof(char));
    strcpy(newString, str);

    stack->top++;
    stack->strings[stack->top] = newString;
}

char* popString(StringStack* stack) {
    if (isStringStackEmpty(stack)) {
        printf("Error: String stack is empty.\n");
        return NULL;
    }

    char* poppedString = stack->strings[stack->top];
    stack->top--;

    return poppedString;
}


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

void cdCommand(task_t* task) {
    
    char buff[512];
    char* filename;
    char path[128];
    
    // 根据username查表获取user表信息，再通过user表信息获取pwd
    char * username = task->user->userName;
    User * user = selectUserByUserName(username);
    char * pwd = user->pwd;


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
        filename = getParentDirectory(pwd);
        printf("要切换到目录名为:%s", filename);
        if (filename == NULL) {
            sprintf(buff, "当前已在根目录!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;

        } else {
            bzero(path,sizeof(path));
            strcpy(path, getParentPath(pwd));
            sprintf(buff, "当前路径为>%s", path);
            sendn(task->peerfd, buff, strlen(buff));
            strcpy(user->pwd, path); // 更新当前所在目录
            updateUser(user); // 将更改保存到user表
            return;
        }
    } else {
        
        // cd 到 mc,mc一定在当前目录下，需要查找当前目录下是否存在对应的文件名
        // 获取当前目录的虚拟文件表
        FileEntry * curr = selectFileEntryByFileName(getCurrentDirectory(pwd));

        // 获取要切换的目录名
        filename = getCurrentDirectory(task->data);
        // 判断切换的目录是否存在
        FileEntry* dir = selectFileEntryByFileNameParentIdOwnerId(filename, curr->id, curr->ownerId);
        
        if (dir == NULL) {
            sprintf(buff, "要切换的目录不存在!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;
        }
        // 目录存在
        // StringStack* stack = createStringStack();
        // pushString(stack, filename);
        // 
      //     while (dir != NULL) {
      //         dir = selectFileEntry(dir->parentId);
      //         if (dir == NULL) break;
      //         pushString(stack, dir->fileName);
      //     }
      //     
      //     while (!isStringStackEmpty(stack)) {
      //         char* str = popString(stack);
      //         strcat(path, "/");
      //         strcat(path, str);
      //     }
        
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
