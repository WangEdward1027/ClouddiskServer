#ifndef __WD_FUNC_H
#define __WD_FUNC_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/sendfile.h>
#include <syslog.h>
#include <shadow.h>
#include <sys/syslog.h>
#include "linked_list.h"
#include "user.h"
#include <limits.h>
#include <asm-generic/errno-base.h>

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef void (*sighandler_t)(int);

#define ARGS_CHECK(argc, num)   {\
    if(argc != num){\
        fprintf(stderr, "ARGS ERROR!\n");\
        return -1;\
    }}

#define ERROR_CHECK(ret, num, msg) {\
    if(ret == num) {\
        perror(msg);\
        return -1;\
    }}

#define THREAD_ERROR_CHECK(ret, func) {\
    if(ret != 0) {\
        fprintf(stderr, "%s:%s\n", func, strerror(ret));\
    }}

typedef enum {
    CMD_TYPE_PWD = 1,
    CMD_TYPE_LS = 2,
    CMD_TYPE_CD = 3,
    CMD_TYPE_MKDIR = 4,
    CMD_TYPE_RMDIR = 5,
    CMD_TYPE_PUTS = 6,
    CMD_TYPE_GETS = 7,
    CMD_TYPE_TREE = 8,
    CMD_TYPE_LOG = 9,
    CMD_TYPE_NOTCMD = 10,  //不是命令
    CMD_TYPE_TOUCH = 11,  

    TASK_LOGIN_SECTION1 = 100,
    TASK_LOGIN_SECTION1_RESP_OK,
    TASK_LOGIN_SECTION1_RESP_ERROR,
    TASK_LOGIN_SECTION2,
    TASK_LOGIN_SECTION2_RESP_OK,
    TASK_LOGIN_SECTION2_RESP_ERROR
}CmdType;

typedef struct 
{
    int len;//记录内容长度
    CmdType type;
    char buff[1000];//记录内容本身
}train_t;

typedef struct task_s{
    int peerfd;
    CmdType type;
    char data[1000];
    struct task_s * pNext;
}task_t;

typedef struct task_queue_s
{
    task_t * pFront;
    task_t * pRear;
    int queSize;//记录当前任务的数量
    pthread_mutex_t mutex; 
    pthread_cond_t cond;
    int flag;//0 表示要退出，1 表示不退出

}task_queue_t;

typedef struct threadpool_s {
    pthread_t * pthreads;
    int pthreadNum;
    task_queue_t que;//...任务队列
}threadpool_t;

int queueInit(task_queue_t * que);
int queueDestroy(task_queue_t * que);
int queueIsEmpty(task_queue_t * que);
int taskSize(task_queue_t * que);
int taskEnque(task_queue_t * que, task_t * task);
task_t * taskDeque(task_queue_t * que);
int broadcastALL(task_queue_t* que);

int threadpoolInit(threadpool_t *, int num);
int threadpoolDestroy(threadpool_t *);
int threadpoolStart(threadpool_t *);
int threadpoolStop(threadpool_t *);


int tcpInit(const char * ip, const char * port);
int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);
int transferFile(int sockfd);
int sendn(int sockfd, const void * buff, int len);
int recvn(int sockfd, void * buff, int len);

//处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que);

//执行任务的总的函数
void doTask(task_t * task);
//每一个具体命令的执行
void cdCommand(task_t * task);
void lsCommand(task_t * task);
void pwdCommand(task_t * task);
void mkdirCommand(task_t * task);
void rmdirCommand(task_t * task);
void notCommand(task_t * task);
void putsCommand(task_t * task);
void getsCommand(task_t * task);
void treeCommand(task_t * task);
void logCommand(task_t * task);
/*void getsetting(task_t * task); //获取用户的盐值
void checkUsrEncode(task_t * task); //检验用户输入密码
int sendMessage(int sockfd, char* buffer, CmdType cmdType); //向客户端发送盐值*/
//用户登录的操作
void userLoginCheck1(task_t * task);
void userLoginCheck2(task_t * task);

// 日志
void log_login(char* username); // 记录用户登录信息
void log_action(task_t* task); // 记录用户操作信息


#endif
