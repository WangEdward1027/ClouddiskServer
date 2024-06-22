#include "thread_pool.h"
#define EPOLL_ARR_SIZE 100

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展
//主线程调用
void mainDoTask(task_t * task)
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
    case CMD_TYPE_LOGIN_USRNAME:
        userLoginCheck1(task); break;
    case CMD_TYPE_LOGIN_ENCRYTPTEDCODE:
        userLoginCheck2(task); break;
    case CMD_TYPE_REGISTER_USERNAME:
        userRegister1(task); break;
    case CMD_TYPE_REGISTER_ENCRYTPTEDCODE:
        userRegister2(task); break;
    case CMD_TYPE_PUTS:
    case CMD_TYPE_GETS:
    case MSG_TYPE_LOGIN_SALT:
    case MSG_TYPE_LOGINOK:
    case MSG_TYPE_LOGINERROR:
    case MSG_TYPE_REGISTEROK:
    case MSG_TYPE_REGISTERERROR:
        break;
    case CMD_TYPE_TOUCH:
        touchCommand(task);   break;
    case CMD_TYPE_REMOVE:
        removeCommand(task);  break;
    default:
        break;
    }
}


//子线程调用，专门处理长命令
void doTask(task_t * task)
{
    //先建立建立自动递增的静态监听端口
    ++port;
    char PORT[8];
    sprintf(PORT, "%d", port);
    //创建监听套接字
    int listenfd = tcpInit("127.0.0.1", PORT);

    //创建epoll实例
    int epfd = epoll_create1(0);

    //对listenfd进行监听
    //再将连接加入监听
    addEpollReadfd(epfd, listenfd);
    //存储已经连接的任务端口
    struct epoll_event * pEventArr = (struct epoll_event*)calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));

    while(1) {
        int nready = epoll_wait(task->epfd, pEventArr, EPOLL_ARR_SIZE, -1);
        if(nready == -1 && errno == EINTR) {
            continue;
        } else if(nready == -1) {
            printf("epoll_wait fiale!\n");
            exit(-1);
        } else {
            //大于0
            for(int i = 0; i < nready; ++i) {
                int fd = pEventArr[i].data.fd;
                if(fd == listenfd) {//对新连接进行处理
                    int peerfd = accept(listenfd, NULL, NULL);
                    printf("\n conn %d has conneted.\n", peerfd);
                    //将新连接添加到epoll的监听红黑树上
                    addEpollReadfd(epfd, peerfd);
                }else{
                    //接收客户端发来的Token

                    //TODO 验证Token

                    //执行对应的任务
                    if(task->type == CMD_TYPE_PUTS)
                    {
                        //上传任务执行前先将监听移除
                        delEpollReadfd(epfd, listenfd);
                        putsCommand(task);   
                        //上传任务执行完毕之后，再加回来
                        addEpollReadfd(epfd, listenfd);
                        break;
                    }else if(task->type == CMD_TYPE_GETS)
                        getsCommand(task);            
                }
            }
        }
    }
}

//主线程调用:处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que)
{
    //消息格式：cmd content
    //1.1 获取消息长度
    int length = -1;
    int ret = recvn(sockfd, &length, sizeof(length));
    if(ret == 0) {
        goto end;
    }
    printf("\n\nrecv length: %d\n", length);

    //1.2 获取消息类型
    CmdType cmdType;
    ret = recvn(sockfd, &cmdType, sizeof(cmdType));
    if(ret == 0) {
        goto end;
    }
    printf("recv cmd type: %d\n", cmdType);

    task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
    ptask->peerfd = sockfd;
    ptask->epfd = epfd;
    ptask->type= cmdType;

    //1.3 获取用户信息
    //ptask->user
    if(ptask->type!=CMD_TYPE_REGISTER_USERNAME&&ptask->type!=CMD_TYPE_REGISTER_ENCRYTPTEDCODE)
    {
        User* user = (User*)calloc(1 ,sizeof(User));
        ret = recvn(sockfd, user, sizeof(User));
        printf("ret:%d\n",ret);
        if(ret == 0)
            goto end;
        printf("User:usrname=%s, salt=%s, cryptpasswd=%s, pwd=%s\n",
               user->userName, user->salt, user->cryptpasswd, user->pwd);
        ptask->user = user;
    }

    if(length > 0) {
        if(ptask->type==CMD_TYPE_REGISTER_USERNAME||ptask->type==CMD_TYPE_REGISTER_ENCRYTPTEDCODE)
        {
            User* user = (User*)calloc(1 ,sizeof(User));
            ret = recvn(sockfd, user, sizeof(User));
            printf("ret:%d\n",ret);
            if(ret == 0)
                goto end;
            printf("User:usrname=%s, salt=%s, cryptpasswd=%s, pwd=%s\n",
                   user->userName, user->salt, user->cryptpasswd, user->pwd);
            ptask->user = user;
        }else{
            //1.4 获取参数消息内容
            ret = recvn(sockfd, ptask->data, length);
        }

        if(ret > 0) {
            //若为puts和gets命令就往线程池中添加任务
            if(ptask->type == CMD_TYPE_PUTS || ptask->type == CMD_TYPE_GETS)
                taskEnque(que, ptask);
            else //直接调用mainDotask
                mainDoTask(ptask);
        }
    }else if (length == 0){
        //短命令，直接调用mainDoTask
        mainDoTask(ptask);
    }


end:
    if(ret == 0) {//连接断开的情况
        printf("\nconn %d is closed.\n", sockfd);
        delEpollReadfd(epfd, sockfd);
        close(sockfd);
        /* deleteNode2(&userList, sockfd);//删除用户信息 */
    }
}

void notCommand(task_t *task){
    //服务器判断不是正常的指令, 给客户端发回提示
    char buff[128] = "This is not a command.\n";
    buff[strlen(buff)+1] = '\0';
    sendn(task->peerfd, buff , strlen(buff) + 1);
}

//对指定文件生成响应的MD5值
void generateMD5(const char *filename, char *md5String) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;
    FILE *file;
    int bytes;
    unsigned char data[1024];

    // 初始化上下文
    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        printf("EVP_MD_CTX_new failed\n");
        return;
    }

    // 使用MD5算法
    md = EVP_md5();
    if (EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
        printf("EVP_DigestInit_ex failed\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // 打开文件
    file = fopen(filename, "rb");
    if (file == NULL) {
        printf("%s can't be opened.\n", filename);
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // 读取文件并更新MD5上下文
    while ((bytes = fread(data, 1, 1024, file)) != 0) {
        if (EVP_DigestUpdate(mdctx, data, bytes) != 1) {
            printf("EVP_DigestUpdate failed\n");
            fclose(file);
            EVP_MD_CTX_free(mdctx);
            return;
        }
    }

    // 完成哈希计算
    if (EVP_DigestFinal_ex(mdctx, md_value, &md_len) != 1) {
        printf("EVP_DigestFinal_ex failed\n");
        fclose(file);
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // 释放上下文
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    // 将MD5值转换为十六进制字符串
    for (i = 0; i < md_len; i++) {
        sprintf(&md5String[i * 2], "%02x", md_value[i]);
    }
    md5String[md_len * 2] = '\0';
}
