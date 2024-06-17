#include "thread_pool.h"

//主线程调用:处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que)
{
    //消息格式：cmd content
    //1.1 获取消息长度
    int length = -1;
    int ret = recvn(sockfd, &length, sizeof(length));
    printf("recv length: %d\n", length);

    //1.2 获取消息类型
    CmdType cmdType;
    ret = recvn(sockfd, &cmdType, sizeof(cmdType));
    printf("recv cmd type: %d\n", cmdType);
    task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
    ptask->peerfd = sockfd;
    ptask->type= cmdType;
    if(length > 0) {
        //1.3 获取消息内容
        ret = recvn(sockfd, ptask->data, length);
        if(ret > 0) {
            //往线程池中添加任务
            taskEnque(que, ptask);
        }
    } else if(length == 0){
        taskEnque(que, ptask);
    }

    if(ret == 0) {//连接断开的情况
        printf("\nconn %d is closed.\n", sockfd);
        delEpollReadfd(epfd, sockfd);
        close(sockfd);
    }
}

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展
void doTask(task_t * task)
{
    assert(task);
    switch(task->type) {
    case CMD_TYPE_PWD:  
        pwdCommand(task);   break;
    case CMD_TYPE_CD:
        cdCommand(task);    break;
    case CMD_TYPE_LS:
        lsCommand(task);    break;
    case CMD_TYPE_MKDIR:
        mkdirCommand(task);  break;
    case CMD_TYPE_RMDIR:
        rmdirCommand(task);  break;
    case CMD_TYPE_NOTCMD:
        notCommand(task);   break;
    case CMD_TYPE_PUTS:
        putsCommand(task);   break;
    case CMD_TYPE_GETS:
        getsCommand(task);   break;
    case CMD_TYPE_TREE:
        treeCommand(task);   break;
    case CMD_TYPE_USRNAME:
        getsetting(task);        break;
    case CMD_TYPE_USRENCODE:
        checkUsrEncode(task);    break;
    default:
        break;
    }
}
void cdCommand(task_t * task)
{
    // 保存用户要切换的目录
    char* path = task->data;
    char buff[1024];
    bzero(buff, sizeof(buff));

    // 切换到相应的目录
    if (chdir(path) == -1) {
        // 切换失败
        error(1, errno, "chdir %s failed!\n", path);
    }

    // 切换成功，打印新的当前工作目录
    char cwd[1024];
    bzero(cwd, sizeof(cwd));

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // 获取当前工作目录成功
        sprintf(buff, "切换目录成功！当前目录为：%s\n", cwd);
    }

    // 发送buff到客户端
    sendn(task->peerfd, buff, sizeof(buff));
}
void lsCommand(task_t * task)
{
    char* cwd; // 用于保存用户传入的目录
    char buff[1024]; // 用于保存要发送给客户端的内容
    bzero(&buff, sizeof(buff));

    if (strcmp(task->data, "") == 0) {
        // 用户未传入任何内容

        cwd = getcwd(NULL, 0); // 获取当前工作目录的绝对路径

        if (cwd == NULL) {
            // 获取当前目录失败
            error(1, errno, "getcwd failed");
        }
    }else {
        cwd = task->data;
    }


    DIR* stream = opendir(cwd); // 打开当前所在目录的目录流
    if (!stream) {
        error(1, errno, "opendir %s", cwd);
    }

    struct dirent* pdirent; // 结构体用于保存当前目录信息

    int i = 0;

    while((pdirent = readdir(stream)) != NULL) {
        char* filename = pdirent->d_name;

        // 忽略.和..
        if ((strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)) {
            continue;
        }

        // 忽略隐藏文件
        if (filename[0] == '.') {
            continue;
        }


        // 打印目录项名字
        // puts(filename); 这里不用puts的原因是 puts属于行缓冲，
        // 每次输出都会在末尾自动加一个换行符，不符合ls命令的形式
        // printf("%-15s\t", filename);
        sprintf(buff, "%-15s\t", filename);

        i++;
        if (i % 4 == 0) {
            // printf("\n");   // 为了美观设置每输出4个单词就换行
            sprintf(buff, "\n");
        }

    }

    // printf("\n");
    sprintf(buff, "\n");

    closedir(stream);

    // 将服务器端ls产生的内容，发送到客户端
    sendn(task->peerfd, buff, sizeof(buff));

}
void pwdCommand(task_t * task)
{
    char* cwd;
    char buff[1024] = {0};
    bzero(buff, sizeof(buff));
    if ((cwd = getcwd( NULL, 0)) == NULL) {
        error( 1, errno, "getcwd failed");
    }

    // 将获取的pwd保存到buff中
    strcpy(buff, cwd);

    free(cwd);

    // 将buff的内容发送给客户端
    sendn(task->peerfd, buff, sizeof(buff));

}
void mkdirCommand(task_t * task)
{
    char *message1="Create dir Sucessful!";
    char *message2="Create dir Failed!";
    printf("Execute mkdir command.");

    train_t t;
    memset(&t,0,sizeof(t));//初始化

    if(mkdir( task->data , 0777) ==-1){
        strcpy(t.buff,message2);
        t.len=strlen(t.buff);
        sendn(task->peerfd,&t,t.len+4);
        syslog(LOG_WARNING,"User execute MKDIR command: Failed.");
    }
    else{
        strcpy(t.buff,message1);
        t.len=strlen(t.buff);
        sendn(task->peerfd,&t,t.len+4);
        syslog(LOG_INFO,"User execute MKDIR command: Successful.");
    }

}

void rmDirRec(const char *path)
{
    //打开目录
    DIR *dp=opendir(path);
    if(dp==NULL)
    {
        error(1,errno,"opendir");
    }
    //遍历目录流，递归删除每一项目录项
    errno=0;
    struct dirent*pdir;
    while((pdir=readdir(dp))!=NULL){
        //忽略.和..
        char *name =pdir->d_name;
        if(strcmp(name,".")==0||strcmp(name,"..")==0){
            continue;
        }

        //拼接路径
        char subpath[1024];
        sprintf(subpath,"%s/%s",path,name);

        //递归删除
        if(pdir->d_type==DT_DIR){
            rmDirRec(subpath);
        }else if(pdir->d_type==DT_REG){
            unlink(subpath);
            /*对于硬链接来说，unlink 用来删除目录项，并把 inode 引用计数减 1，这两步也是一个原子过程。
             * 直到 inode 引用计数为 0，才会真正删除文件。
             对于软链接来说，unlink 直接删除软链接，而不影响软链接指向的文件。*/
        }
    }//pdir==NULL
    closedir(dp);

    if(errno){
        error(1,errno,"readdir");
    }
    //再删除目录
    rmdir(path);
}

void rmdirCommand(task_t * task)
{
    //接收任务中的目录
    char dirPath[1000];
    strncpy(dirPath,task->data,sizeof(dirPath)-1);
    dirPath[sizeof(dirPath)-1]='\0';//确保字符串以null结尾

    //递归删除函数
    rmDirRec(dirPath);

}
void notCommand(task_t * task)
{
    ;
}
void putsCommand(task_t * task);
void getsCommand(task_t * task);
void treeCommand(task_t * task);
void getsetting(task_t * task); //获取用户的盐值
void checkUsrEncode(task_t * task); //检验用户输入密码
int sendMessage(int sockfd, char* buffer, CmdType cmdType); //向客户端发送盐值


