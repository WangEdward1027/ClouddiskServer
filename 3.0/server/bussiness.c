#include "linked_list.h"
#include "thread_pool.h"

//外部变量(userList是在main.c文件中定义的)
extern ListNode * userList;

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

    //TODO ptask->fileEntry


    if(length > 0) {
        //1.3 获取消息内容
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
        deleteNode2(&userList, sockfd);//删除用户信息
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
        notCommand(task);   break;
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


