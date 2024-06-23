#include "thread_pool.h"


void mkdirCommand(task_t * task) {

    char * dirname; // 用于暂存要插入的字符
    char buff[512] = {0}; // 保存给用户发送的内容

    // 1. 先通过当前的username获取用户表信息，从而获取pwd
    char * username = task->user->userName;
    User * user = selectUserByUserName(username);
    char * pwd = user->pwd;
    

    // 移除指令后的空格
    removeTrailingSpace(task->data);

    // 2. 获取当前目录的虚拟文件表
    int num = 0;
    FileEntry * curr = selectFileEntryByFileNameAndOwnerId(getCurrentDirectory(pwd), user->id, &num);
    
    // 3. 查询当前目录下是否已存在同名目录
    dirname = task->data;
    FileEntry * dir = selectFileEntryByFileNameParentIdOwnerId(dirname, curr->id, user->id);
    if (dir) {
        // dir存在，说明已存在同名目录，返回
        sprintf(buff, "%s已存在", dirname);
        strcat(buff, "\0");
        sendn(task->peerfd, buff, sizeof(buff));
        return;
    }
    // dir不存在，构建dir的信息

    FileEntry * newdir = (FileEntry*)calloc(1, sizeof(FileEntry));
    newdir->parentId = curr->id;
    strcpy(newdir->fileName, dirname);
    newdir->ownerId = user->id;
    strcpy(newdir->md5, "0");
    newdir->fileSize = 0;
    newdir->fileType = 0;

    int ret = addFileEntry(newdir);
    if (ret == -1) {
        sprintf(buff, "创建失败！");
        sendn(task->peerfd, buff, sizeof(buff));
        return;
    } 
    
    sprintf(buff, "创建成功！");
    sendn(task->peerfd, buff, sizeof(buff));

}
