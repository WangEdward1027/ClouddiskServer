 #include "thread_pool.h"

// 服务端
void putsCommand(task_t * task) {
    
    char response[64] = {0};

    // 1. 服务端接收客户端的md5
    char md5String[33];
    recvn(task->peerfd, md5String, sizeof(md5String));
    printf("收到的md5值是:%s\n", md5String);
    removeTrailingSpace(task->data);

    // 2. 查表判断是否已存在md5值相同的文件，存在发送1，不存在返回0
    FileInfo * file = selectFileInfo(md5String, 32);
    int ret = -1; // 标记文件是否存在
    if (file) {
        // 2.1.1 文件存在，给客户端发送ret = 1
        ret = 1;
        sendn(task->peerfd, &ret, sizeof(int));
        
        // 2.1.2 在虚拟文件表插入文件信息
        // 找到当前目录所在位置文件表
        // 根据username获取当前user表信息，通过user表信息获取pwd
        char * username = task->user->userName;
        User * user = selectUserByUserName(username);
        char * pwd = user->pwd;
        
        // 根据pwd,查找到当前所在的目录的虚拟文件表信息
        char tmp[64] = {0};
        strcpy(tmp, pwd);
        int num = 0;
        FileEntry * curr = selectFileEntryByFileNameAndOwnerId(getCurrentDirectory(tmp), user->id, &num);
        
        // 填充新文件的FileEntry信息
        FileEntry * newfile = (FileEntry*)calloc(1, sizeof(FileEntry));
        newfile->parentId = curr->id;
        strcpy(newfile->fileName, task->data);
        newfile->ownerId = user->id;
        strcpy(newfile->md5, md5String);
        newfile->fileSize = 0;
        newfile->fileType = 1;
            

        // 向客户端发送返回结果
        int ret = addFileEntry(newfile);
        printf("执行到了这里!\n");
        if (ret == -1) {
            sprintf(response, "上传文件失败!");
            sendn(task->peerfd, response, sizeof(response));
            return;
        }
        
        printf("2执行到了这里!\n");

        sprintf(response, "秒传，上传文件成功!");
        int reslength = strlen(response);
        sendn(task->peerfd, &reslength, sizeof(int));
        sendn(task->peerfd, response, reslength);
        
        printf("3执行到了这里!\n");
        return;
    }

    // 3. 文件不存在
    // 发送ret结果
    ret = 0;
    sendn(task->peerfd, &ret, sizeof(ret));
    // 3.1 获取客户端发送的文件大小
    long int filelength = 0;
    recvn(task->peerfd, &filelength, sizeof(filelength));
    printf("接受到文件大小为%ld\n", filelength);

    // 3.2 新建文件，写入数据
    int fd = open(task->data, O_WRONLY | O_CREAT, 0664);
    if (fd < 0) {
        fprintf(stderr, "文件打开失败\n");
    }

    ret = 0;
    char buff[1000];
    off_t left = filelength;
    while (left > 0) {
        if (left < 1000) {
            ret = recvn(task->peerfd, buff, left); 
        } else {
            ret = recvn(task->peerfd, buff, sizeof(buff));
        }
        if (ret < 0) {
            break;
        }
        ret = write(fd, buff, ret);
        left -= ret;
    }
    close(fd);

    // 3.3 将信息写入数据库
    // 3.3.1 将信息写入文件表
    FileInfo * newinfo = (FileInfo*)calloc(1, sizeof(FileInfo));
    strcpy(newinfo->md5, md5String);
    strcpy(newinfo->fileName, task->data);
    addFileInfo(newinfo);

    // 3.3.2 将文件信息写入虚拟文件表
    // 找到当前目录所在位置文件表
    // 根据username获取当前user表信息，通过user表信息获取pwd
    char * username = task->user->userName;
    User * user = selectUserByUserName(username);
    char * pwd = user->pwd;
        
    // 根据pwd,查找到当前所在的目录的虚拟文件表信息
    char tmp[64] = {0};
    strcpy(tmp, pwd);
    int num = 0;
    FileEntry * curr = selectFileEntryByFileNameAndOwnerId(getCurrentDirectory(tmp), user->id, &num);
        
    // 填充新文件的FileEntry信息
    FileEntry * newfile = (FileEntry*)calloc(1, sizeof(FileEntry));
    newfile->parentId = curr->id;
    strcpy(newfile->fileName, task->data);
    newfile->ownerId = user->id;
    strcpy(newfile->md5, md5String);
    newfile->fileSize = filelength;
    newfile->fileType = 1;
     
    // 向客户端发送返回结果
    ret = addFileEntry(newfile);
    if (ret == -1) {
        sprintf(response, "上传文件失败!");
        sendn(task->peerfd, response, sizeof(response));
    }
        
    sprintf(response, "写入文件,上传文件成功!");
    sendn(task->peerfd, response, sizeof(response));

}
