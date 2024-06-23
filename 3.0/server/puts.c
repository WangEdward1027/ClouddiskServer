#include "thread_pool.h"

void putsCommand_old(task_t * task) {
    
    removeTrailingSpace(task->data);
    //秒传功能:
    //服务器先接收md5码, 对比文件表中是否存在
    char md5String[33];
    sscanf(task->data, "%s", md5String);

    //若存在, 实现秒传
    if(selectFileInfo(md5String, 33)){
        strncpy(task->data, "instantTransfer", sizeof(task->data));
        send(task->peerfd, task->data, strlen(task->data), 0);
        
        //插入虚拟文件表    
        FileEntry fileEntry;
        //parentId
        char* curdir = getCurrentDirectory(task->user->pwd);
        int count;
        //根据fileName和ownerId查询表，并填好结构体
        FileEntry* fileEntry_parent = selectFileEntryByFileNameAndOwnerId(curdir, task->user->id, &count);
        fileEntry.parentId = fileEntry_parent->id;

        //fileName  //客户端传输md5,还要传文件名
        /* strcpy(fileEntry.fileName, ); */
        fileEntry.ownerId = task->user->id;
        strcpy(fileEntry.md5, md5String);    
        //fileSize      //查虚拟文件表的md5码，得到文件的fileSize
        fileEntry.fileType = 1; //1是文件
        addFileEntry(&fileEntry);

        return;
    }
    //若不存在, 开始普通上传
    printf("文件不存在服务器，开始普通上传文件.\n");
    char filename[20] = {0};
    strcpy(filename, task->data);
    printf("filname: %s\n", filename);
    off_t len = 0;
    int ret = recvn(task->peerfd, &len, sizeof(len));
    printf("filelen: %ld.\n", len);

    //打开文件
    int fd = open(filename, O_CREAT|O_RDWR, 0644);
    if(fd < 0) {
        perror("open"); return;
    }
    //接收并写入文件
    char buff[1000] = {0};
    off_t left = len;
    while(left > 0) {
        if(left < 1000) {
            ret = recvn(task->peerfd, buff, left);
        } else {
            ret = recvn(task->peerfd, buff, sizeof(buff));
        }
        if(ret < 0) {
            break;
        }
        ret = write(fd, buff, ret);
        left -= ret;
    }
    close(fd);
    
    //将这个新文件插入服务器文件表
    FileInfo fileInfo;
    strcpy(fileInfo.md5, md5String);
    strcpy(fileInfo.fileName, filename);
    addFileInfo(&fileInfo);

    //插入虚拟文件表
    FileEntry fileEntry;
    //parentID
    char* curdir = getCurrentDirectory(task->user->pwd);
    int count;
        //根据fileName和ownerId查询表，并填好结构体
    FileEntry* fileEntry_parent = selectFileEntryByFileNameAndOwnerId(curdir, task->user->id, &count);
    fileEntry.parentId = fileEntry_parent->id;
    
    strcpy(fileEntry.fileName, filename);
    fileEntry.ownerId = task->user->id;
    strcpy(fileEntry.md5, md5String);
    fileEntry.fileSize = len;
    fileEntry.fileType = 1;  //0是目录，1是文件
    ret = addFileEntry(&fileEntry);
    if(ret != 0){
        printf("puts.c addFileEntry插入失败\n");
        exit(1);
    }

    //上传任务执行完毕之后，再加回来
    addEpollReadfd(task->epfd, task->peerfd);
    return;
}
