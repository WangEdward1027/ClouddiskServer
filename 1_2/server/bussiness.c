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

void notCommand(task_t *task){
     //先随便写一个
     printf("not this command");
     //实际应该发回给客户端提示
     //...
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
