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
            //puts命令预处理
            if(ptask->type == CMD_TYPE_PUTS) {
                //是上传文件任务，就暂时先从epoll中删除监听
                delEpollReadfd(epfd, sockfd);
            }
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

void notCommand(task_t *task){
    //服务器判断不是正常的指令, 给客户端发回提示
    char buff[128] = "This is not a command.\n";
    buff[strlen(buff)+1] = '\0';
    sendn(task->peerfd, buff , strlen(buff) + 1); 
}

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展
void doTask(task_t * task)
{
    assert(task);
    log_action(task);
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
        //上传任务执行完毕之后，再加回来
        addEpollReadfd(task->peerfd, task->peerfd);
        break;
    case CMD_TYPE_GETS:
        getsCommand(task);   break;
    case CMD_TYPE_TREE:
        treeCommand(task);   break;
    case CMD_TYPE_LOG:
        logCommand(task);   break;
    case TASK_LOGIN_SECTION1:
        userLoginCheck1(task); break;
    case TASK_LOGIN_SECTION2:
        userLoginCheck2(task); break;

    default:
        break;
    }
}

//外部变量(userList是在main.c文件中定义的)
extern ListNode * userList;

void userLoginCheck1(task_t * task) {
    printf("userLoginCheck1.\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_info_t * user = (user_info_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            //拷贝用户名
            strcpy(user->name, task->data);
            loginCheck1(user);
            return;
        }
    }
}

void userLoginCheck2(task_t * task) {
    printf("userLoginCheck2.\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_info_t * user = (user_info_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            //拷贝加密密文
            loginCheck2(user, task->data);
            return;
        }
    }
}
