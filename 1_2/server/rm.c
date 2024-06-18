/*甘博*/

#include"thread_pool.h"

void delete_directory(const char *directory_path) {
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];
    struct stat statbuf;

    // 打开目录
    dir = opendir(directory_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    // 遍历目录中的每个子项
    while ((entry = readdir(dir)) != NULL) {
        // 忽略当前目录（"."）和上级目录（".."）
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构建子项的完整路径
        snprintf(path, sizeof(path), "%s/%s", directory_path, entry->d_name);

        // 获取子项的信息
        if (lstat(path, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        // 递归删除子目录
        if (S_ISDIR(statbuf.st_mode)) {
            delete_directory(path);
        }
        // 删除文件
        else {
            if (unlink(path) == -1) {
                perror("unlink");
                continue;
            }
        }
    }

    // 关闭目录
    closedir(dir);

    // 删除空目录
    if (rmdir(directory_path) == -1) {
        perror("rmdir");
    } else {
        printf("目录 '%s' 已成功删除。\n", directory_path);
    }
}

void rmdirCommand(task_t *task){
    //接收任务中的目录
    if(strlen(task->data)==0){
        const char *msg = "请输入目录";
        sendn(task->peerfd, msg, strlen(msg));
        return;
    }
    char dirPath[1000];
    strncpy(dirPath,task->data,sizeof(dirPath)-1);
    dirPath[sizeof(dirPath)-1]='\0';//确保字符串以null结尾
    
    
    DIR *testp=opendir(dirPath);
    if(testp==NULL){
        const char *msg = "请输入目录";
        sendn(task->peerfd, msg, strlen(msg));
        return;
    }

    //递归删除函数
    delete_directory(dirPath);

     // 删除成功信息
    const char *successMsg = "删除成功";
    sendn(task->peerfd, successMsg, strlen(successMsg));
}
