/* #include "linked_list.h" */
#include "thread_pool.h"

//外部变量(userList是在main.c文件中定义的)
/* extern ListNode * userList; */

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
    User* user = (User*)calloc(1 ,sizeof(User));
    ret = recvn(sockfd, user, sizeof(User));
    if(ret == 0)
        goto end;
    printf("User:usrname=%s, salt=%s, cryptpasswd=%s, pwd=%s",
           user->userName, user->salt, user->cryptpasswd, user->pwd);
    ptask->user = user;

    if(length > 0) {
        //1.4 获取消息内容
        ret = recvn(sockfd, ptask->data, length);
        if(ret > 0) {
            //往线程池中添加任务
            if(ptask->type == CMD_TYPE_PUTS) {
                //是上传文件任务，就暂时先从epoll中删除监听
                delEpollReadfd(epfd, sockfd);
            }
            taskEnque(que, ptask);
        }
    } else if(length == 0){
        taskEnque(que, ptask);
    }
end:
    if(ret == 0) {//连接断开的情况
        printf("\nconn %d is closed.\n", sockfd);
        delEpollReadfd(epfd, sockfd);
        close(sockfd);
        /* deleteNode2(&userList, sockfd);//删除用户信息 */
    }
}

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展
//子线程调用
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
    case CMD_TYPE_PUTS:
        putsCommand(task);   break;
        //上传任务执行完毕之后，再加回来
        addEpollReadfd(task->peerfd, task->peerfd);
        break;
    case CMD_TYPE_GETS:
        getsCommand(task);   break;
    case CMD_TYPE_NOTCMD:
        /* notCommand(task);   break; */
    case CMD_TYPE_LOGIN_USRNAME:
        userLoginCheck1(task); break;
    case CMD_TYPE_LOGIN_ENCRYTPTEDCODE:
        userLoginCheck2(task); break;
    case CMD_TYPE_REGISTER_USERNAME:
        userRegister1(task); break;
    case CMD_TYPE_REGISTER_ENCRYTPTEDCODE:
        userRegister2(task); break;
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
