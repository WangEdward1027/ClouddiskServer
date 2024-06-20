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


void cdCommand(task_t* task) {
    
    char buff[512];
    char* filename;
    char path[128];

    // 1. 判断cd 参数是否合法
    if (strcmp(task->data, "") == 0) {
        strcpy(buff, "请输入正确的cd 命令!\n");
        strcat(buff, "\0");
        sendn(task->peerfd, buff, strlen(buff));
        return;

    } else if (strcmp(task->data, ".") == 0 || strcmp(task->data, "./") == 0) {
        sprintf(buff, ">%s", task->user->pwd);
        sendn(task->peerfd, buff, strlen(buff));
        return;

    } else if (strcmp(task->data, "../") == 0 || strcmp(task->data, "..") == 0) {
        filename = getParentDirectory(task->user->pwd);
        if (filename == NULL) {
            sprintf(buff, "当前已在根目录!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;

        } else {
            strcpy(path, getParentPath(task->user->pwd));
            sprintf(buff, ">%s", path);
            sendn(task->peerfd, buff, strlen(buff));
            strcpy(task->user->pwd, path); // 更新当前所在目录
            return;
        }
    } else {
        filename = getCurrentDirectory(task->data);
        // 判断切换的目录是否存在
        int resnum = 1;
        FileEntry* dir = selectFileEntryByFileNameAndOwnerId(filename, task->user->id, &resnum);
        
        if (dir == NULL) {
            sprintf(buff, "要切换的目录不存在!\n");
            sendn(task->peerfd, buff, strlen(buff));
            return;
        }
        // 目录存在
        StringStack* stack = createStringStack();
        pushString(stack, filename);
        
        while (dir != NULL) {
            dir = selectFileEntry(dir->parentId);
            if (dir == NULL) break;
            pushString(stack, dir->fileName);
        }
        
        while (!isStringStackEmpty(stack)) {
            char* str = popString(stack);
            strcat(path, "/");
            strcat(path, str);
        }

        sprintf(buff, ">%s", path);
        sendn(task->peerfd, buff, strlen(buff));

        // 更新当前path
        strcpy(task->user->pwd, path);
        
        return;
    }
}
